/*
 * main.c
 * 
 *
 * 03 nov 2022 -- 08:25 -03
 * Notes:
 */

/*

MEMORY MAP / RAM POOL LAYOUT
----------------------------

    ----------
    |        | NBASE_MAX_RAM_SIZE-1
    |        |
    |  code  |
    |        |
    ----------
    |        | NBASE_MAX_DATA_AREA_SIZE-1
    |        |
    |        |
    ----------
    |        | NBASE_MAX_VARS_AREA_SIZE-1
    |        |
    |        |
    ---------- 0



VARIABLES AREA FORMAT -> VARIABLE DESCRIPTOR
--------------------------------------------

Each variable descriptor in the variables area occupies 8 bytes as follows:

   | variable_name | obj_mark | data_offset |
   63              23         22            0

   variable_name (5 bytes, including type sentinel).
   obj_mark (1 bit) = This marks the descriptor as pointing to an object.
   data_offset (23 bits) = Offset in the data area.

In data area:

 * An integer occupies the size of the default integer type.
 * A float occupies the size of the default floating point type.
 * A string occupies the size of a pointer, which points to a dynamic object.

*/

/* -------------------------------------------------------------------------------- */
/* Compilation process macros. */

#undef NBASE_DEBUG
#define NBASE_DEBUG

/* Output malloc/free debugging info to standard console output. */ 
/* #define NBASE_DEBUG_MALLOC */
/* Output garbage collector's debugging info to standard console output. */ 
/* #define NBASE_DEBUG_GC */
/* Enable color escape sequences to standard console output. */
#define NBASE_USE_ESCAPE_ANSI_COLORS

/* Enable setjmp/longjmp error recovering. */
#define NBASE_ENABLE_LONGJMP_ERROR_RECOVERY

/* Enable/disable features or commands. */
#define NBASE_INCLUDE_FEATURE_DEBUGTOOLS

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fenv.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef NBASE_ENABLE_LONGJMP_ERROR_RECOVERY
#include <setjmp.h>
#define NBASE_TRY             do { int except_code; if(!(except_code = setjmp(g_state.except_buff))) {
#define NBASE_CATCH           } else {
#define NBASE_ENDTRY          }} while(0);
#define NBASE_THROW(errcod)   longjmp(g_state.except_buff, errcod);
#else
#error No error recovery mechanism available.
#endif /* NBASE_ENABLE_LONGJMP_ERROR_RECOVERY */

/* -------------------------------------------------------------------------------- */
/* Intrinsic interpreter macros. */

#define NBASE_VERSION_MAYOR 1
#define NBASE_VERSION_MINOR 0
#define NBASE_VERSION_PATCH 0

/*! Max line length. */
#define NBASE_MAX_LINE_LEN              80
#define NBASE_MAX_LINE_LEN_PLUS_1       (NBASE_MAX_LINE_LEN + 1)
/*#define NBASE_MAX_LINE_LEN_PLUS_2       (NBASE_MAX_LINE_LEN + 2)*/
/*! Max line count. */
/*#define NBASE_MAX_LINES                 256*/
/*! Max token length. */
#define NBASE_MAX_TOKEN_LEN             80
/*! Max variable name length. */
#define NBASE_MAX_VAR_NAME_LEN          5
/*! Max value for array dimension. */
#define NBASE_MAX_ARRAY_DIMENSION       4

/*! Max size of "RAM" pool. */
#define NBASE_MAX_RAM_SIZE              (1024*1024)
/*! Max size of variables area. */
#define NBASE_MAX_VARS_AREA_SIZE        (32*1024)
/*! Max data area size. */
#define NBASE_MAX_DATA_AREA_SIZE        (128*1024)

#define NBASE_VARS_AREA_BASE            g_state.mem_pool
#define NBASE_DATA_AREA_BASE            (NBASE_VARS_AREA_BASE + NBASE_MAX_VARS_AREA_SIZE)
#define NBASE_CODE_AREA_BASE            (NBASE_DATA_AREA_BASE + NBASE_MAX_DATA_AREA_SIZE)

#define NBASE_OBJECTBIT_MASK            0x800000 /* 1 << 23 */
#define NBASE_VARDATA_OFFSET(x)         ((x) & 0x7fffff)

#define NBASE_MALLOC                    malloc
#define NBASE_FREE                      free
#define NBASE_REALLOC                   realloc
#define NBASE_CALLOC                    calloc
#define NBASE_ASSERT                    assert
#define NBASE_PRINT(str)                { printf( str ); fflush(stdout); }
#define NBASE_PRINTF(fmt, ...)          { printf( fmt __VA_OPT__(,) __VA_ARGS__); fflush(stdout); }

#define NBASE_GC_INITIAL_THRESHOLD      50
#define NBASE_OBJECT_GC_MARK            0x80
#define NBASE_OBJECT_TYPE(t)            (t & 0x7f)

#define NBASE_ASSERT_OR_ERROR(cond, err_msg, ...) \
    if( !( cond )) { \
        nbase_error(err_msg __VA_OPT__(,) __VA_ARGS__); }

#define NBASE_ASSERT_OR_INTERNAL_ERROR(cond, err_msg, ...) \
    if( !( cond )) { \
        nbase_internal_error(err_msg __VA_OPT__(,) __VA_ARGS__); }

#define NBASE_AST_NODE(x)               ((nbase_ast_node*)x)

#define NBASE_DECLARE_OBJECT(struct_name) \
    typedef struct _ ## struct_name \
    { \
        uint8_t obj_type; \
        NBASE_OBJECT* next; \

#define NBASE_END_OBJECT(struct_name) \
    } struct_name;

/* -------------------------------------------------------------------------------- */
/* Type definitions. */

/*! Floating point type. */
typedef float                           NBASE_FLOAT;
/*! Integer type. */
typedef int32_t                         NBASE_INTEGER;
/*! Dimension type, type of an array dimension. */
typedef uint8_t                         NBASE_DIMENSION;

#undef NBASE_BOOL
#undef NBASE_TRUE
#undef NBASE_FALSE
#define NBASE_BOOL  char
#define NBASE_TRUE  1
#define NBASE_FALSE 0

/*! Token types. */
typedef enum _nbase_token
{
    nbase_token_NONE = 0, nbase_token_LEXER_ERROR, nbase_token_STRING, nbase_token_INTEGER_LITERAL,
    nbase_token_FLOAT_LITERAL, nbase_token_IDENTIFIER, nbase_token_PLUS = '+', nbase_token_MINUS = '-',
    nbase_token_MUL = '*', nbase_token_DIV = '/', nbase_token_LEFTPAREN = '(', nbase_token_RIGHTPAREN = ')',
    nbase_token_COLON = ':', nbase_token_SEMICOLON = ';', nbase_token_EQ = '=', nbase_token_COMMA = ',',
    nbase_token_POW = '^', nbase_token_PRINT, nbase_token_REM, nbase_token_DIM, nbase_token_LET,
    nbase_token_NEGATED = '%', nbase_token_MOD = 256, nbase_token_LSHIFT, nbase_token_RSHIFT,
    nbase_token_LESS, nbase_token_LESSEQ, nbase_token_GREATER, nbase_token_GREATEREQ,
    nbase_token_NEQUALS, nbase_token_AND, nbase_token_OR, nbase_token_XOR,
    nbase_token_NOT, nbase_token_GC, nbase_token_END, nbase_token_CLEAR,
    nbase_token_LIST, nbase_token_GOTO, nbase_token_LOADLINE,
#ifdef NBASE_INCLUDE_FEATURE_DEBUGTOOLS
    nbase_token_LVAR, nbase_token_STAT, nbase_token_DUMP,
#endif /* NBASE_INCLUDE_FEATURE_DEBUGTOOLS */

    nbase_token_EOL
} nbase_token;

/*! Data types. */
typedef enum _nbase_datatype
{
    nbase_datatype_NONE = 0,
    nbase_datatype_FLOAT,
    nbase_datatype_INTEGER,
    nbase_datatype_STRING
} nbase_datatype;

/*! Keyword definition structure. */
typedef struct _nbase_keyword
{
    char *keyword;
    void (*func)();
    uint16_t token;
} nbase_keyword;

struct _nbase_object;
#define NBASE_OBJECT struct _nbase_object

typedef enum _nbase_state_flag
{
    nbase_state_flag_GC_PENDING =       0x00000001,
    nbase_state_flag_TOKENIZING =       0x00000002
} nbase_state_flag;

/* Interpreter state. */
typedef struct _nbase_state
{
    /* ----- Parsing. ----- */
    
    const char* nextchar;
    uint32_t in_line;
    uint32_t n_lines;
    int32_t paren_level;

    NBASE_INTEGER int32val;
    NBASE_FLOAT fltval;

    char token[NBASE_MAX_TOKEN_LEN + 1];
    nbase_token next_tok;

    /* ----- Interactive interpreter. ----- */

    /*! Input buffer, separated from the general memory pool. */
    char input_buffer[NBASE_MAX_LINE_LEN_PLUS_1];

    /* ----- Runtime. ----- */

    uint8_t* mem_pool;    
    
    uint64_t* vars_limit;
    uint8_t* data_limit;
    uint8_t* code_limit;

    uint32_t gc_marked;
    uint32_t gc_destroyed;
    uint32_t state_flags;

#ifdef NBASE_ENABLE_LONGJMP_ERROR_RECOVERY
    jmp_buf except_buff;
#endif /* NBASE_ENABLE_LONGJMP_ERROR_RECOVERY */
    
    /* ----- Objects. ----- */
    
    /*! Total number of currently allocated objects. */
    int32_t num_objs;
    /*! Number of objects required to trigger a GC. */
    int32_t max_objs;
    NBASE_OBJECT* obj_list;
} nbase_state;

typedef enum _nbase_object_type
{
    nbase_object_type_NONE = 0,
    nbase_object_type_AST_NODE
} nbase_object_type;

typedef struct _nbase_object
{
    uint8_t obj_type;
    NBASE_OBJECT* next;
} nbase_object;

/*! AST node types. */
typedef enum _nbase_ast_type
{
    nbase_ast_type_NONE = 0, nbase_ast_type_FACTOR, nbase_ast_type_BINARY, nbase_ast_type_UNARY,
    nbase_ast_type_VARIABLE, 
    /* NOTE(Pablo): this became an AST node type as part of a test,
        it worked fine, so it's here to stay currently... */
    nbase_ast_type_COMMAND_PRINT
} nbase_ast_type;

struct _nbase_ast_node;

/*! Operation AST node. */
typedef struct _nbase_op
{
    NBASE_OBJECT* lhs;
    NBASE_OBJECT* rhs;
    uint16_t oper;
} nbase_op;

/*! Basic AST node. */
NBASE_DECLARE_OBJECT(nbase_ast_node)
    nbase_ast_type ast_type;
    nbase_datatype data_type;
    union
    {
        char* str_val;
        NBASE_INTEGER int_val;
        NBASE_FLOAT flt_val;
        nbase_op op;
    } u;
    void* extra;
NBASE_END_OBJECT(nbase_ast_node)

/*! Error types. */
typedef enum _nbase_error_type
{
    nbase_error_type_NUMBER_ERROR = 1, nbase_error_type_UNKNOWN_SYMBOL,
    nbase_error_type_DUPLICATED_SYMBOL, nbase_error_type_UNRECOGNIZED_PARSER_TOKEN,
    nbase_error_type_EXPECTED_KEYWORD, nbase_error_type_UNKNOWN_KEYWORD,
    nbase_error_type_INVALID_OPERANDS_UNARY, nbase_error_type_INVALID_OPERANDS_BINARY,
    nbase_error_type_PARENTHESES_MISMATCH, nbase_error_type_EXPECTED_POSITIVE_VAL,
    nbase_error_type_EXPECTED_NEGATIVE_VAL, nbase_error_type_EXPECTED_ZERO_VAL,
    nbase_error_type_EXPECTED_EXPRESSION, nbase_error_type_DIM_NOT_INT,
    nbase_error_type_UNTERMINATED_STRING, nbase_error_type_RESERVED_WORD_USED_FOR_SYMBOL,
    nbase_error_type_EXPECTED_IDENTIFIER, nbase_error_type_EXPECTED_END_OF_LINE,
    nbase_error_type_UNEXPECTED_TOKEN, nbase_error_type_EXPECTED_EQUALS,
    nbase_error_type_EXPECTED_THEN, nbase_error_type_INCOMPATIBLE_ASSIGNMENT,
    nbase_error_type_UNRECOGNIZED_LEXER_TOKEN, nbase_error_type_CANT_OPEN_INPUT_FILE,
    nbase_error_type_DIVISION_BY_ZERO, nbase_error_type_DOM_ERROR_1,
    nbase_error_type_DOM_ERROR_2
} nbase_error_type;

/*! Variable structure. */
typedef struct _nbase_variable
{
    NBASE_INTEGER data_offset;
    NBASE_DIMENSION dims[NBASE_MAX_ARRAY_DIMENSION];
    char name[NBASE_MAX_VAR_NAME_LEN + 1];
    nbase_datatype type;
} nbase_variable;

/* -------------------------------------------------------------------------------- */
/* Prototypes. */

void*               nbase_malloc(uint32_t pNumBytes);
void                nbase_free(void* pPtr);
void                nbase_destroy_object(NBASE_OBJECT* pObj);
void                nbase_gc_mark(NBASE_OBJECT* pObj);
void                nbase_gc_mark_all();
void                nbase_gc_sweep();
void                nbase_gc();
void                nbase_print_info_and_version();
void                nbase_prompt();
NBASE_OBJECT*       nbase_alloc_ast_node(nbase_ast_type pType, nbase_datatype pDataType,
                        const char* pStrVal, NBASE_INTEGER pIntVal, NBASE_FLOAT pFltVal,
                        NBASE_OBJECT* pLhs, NBASE_OBJECT* pRhs,
                        uint16_t pOper, void* pExtra);
void                nbase_destroy_ast_node(NBASE_OBJECT* pNode);
void                nbase_error(nbase_error_type pErrType, const char* pSrcFile, const char* pSrcFunc,
                        uint32_t pSrcLine, const char* pMsg, ...);
void                nbase_internal_error(const char *pMsg,
                        const char* pSrcFile, const char* pSrcFunc, uint32_t pSrcLine, ...);
int32_t             nbase_get_next_token(NBASE_BOOL pInterpret, NBASE_BOOL pVariableName);
void                nbase_reset_state();

/* -------------------------------------------------------------------------------- */

#define NBASE_IMPLEMENTATION

#ifdef NBASE_IMPLEMENTATION

static const char* THIS_FILE = "main.c";

/*! Type names. */
char *g_nbase_type_names[] =
{
    "UNKNOWN",
    "FLOAT",
    "INTEGER",
    "STRING"
};

const char* g_nbase_error_messages[] = {
    "(PARSERR1) error when parsing number",
    "(PARSERR2) unknown symbol",
    "(PARSERR2.1) duplicated symbol",
    "(PARSERR3) unrecognized parser token",
    "(PARSERR4) expected keyword",
    "(PARSERR5) unknown keyword",
    "(PARSERR6) invalid operands to unary",
    "(PARSERR7) invalid operands to binary",
    "(PARSERR8) parentheses mismatch",
    "(PARSERR9) expected positive value",
    "(PARSERR10) expected negative value",
    "(PARSERR11) expected zero",
    "(PARSERR12) expected expression",
    "(PARSERR13) expected integer expression for array dimension",
    "(PARSERR14) unterminated string",
    "(PARSERR15) reserved word used for symbol",
    "(PARSERR16) expected identifier",
    "(PARSERR17) expected end of line",
    "(PARSERR18) unexpected token",
    "(PARSERR19) expected '='",
    "(PARSERR19.1) expected 'THEN'",
    "(PARSERR20) incompatible types in assignment",
    "(LEXERR1) unrecognized lexer token",
    "(OTHERERR1) can't open input file",
    "(MATHERR1) division by zero",
    "(MATHERR2) dom error 1, FE_INVALID raised",
    "(MATHERR3) dom error 2, FE_DIVBYZERO raised"
};

/*! Global interpreter state. */
nbase_state g_state;
/*! Temporal buffer. */
char g_temp_buff[NBASE_MAX_LINE_LEN_PLUS_1];

/* ******************************************************************************** */

void* nbase_malloc(uint32_t pNumBytes)
{
    void* ptr = NULL;

    NBASE_ASSERT((ptr = NBASE_MALLOC(pNumBytes)));

#ifdef NBASE_DEBUG_MALLOC
    printf(NBASE_PRINT_GREEN"nbase_malloc: num_bytes= %u, ptr= %p\n"NBASE_PRINT_DEFAULT, pNumBytes, ptr);
#endif /* NBASE_DEBUG_MALLOC */

    return ptr;
}

/* ******************************************************************************** */

void nbase_free(void* pPtr)
{
#ifdef NBASE_DEBUG_MALLOC
    if(pPtr)
        printf(NBASE_PRINT_RED"nbase_free: ptr= %p\n"NBASE_PRINT_DEFAULT, pPtr);
#endif /* NBASE_DEBUG_MALLOC */
    
    NBASE_FREE(pPtr);
}

/* ******************************************************************************** */

void nbase_destroy_object(NBASE_OBJECT* pObj)
{
    NBASE_ASSERT(pObj);
    
    switch(NBASE_OBJECT_TYPE(pObj->obj_type))
    {
    case nbase_object_type_AST_NODE:
        nbase_destroy_ast_node(pObj);
        break;
    
    default:
        NBASE_ASSERT_OR_INTERNAL_ERROR(0,
            "UNKNOWN OBJECT TYPE", THIS_FILE, __FUNCTION__, __LINE__);
    }

    g_state.num_objs--;
}

/* ******************************************************************************** */

void nbase_gc_mark(NBASE_OBJECT* pObj)
{
    pObj->obj_type |= NBASE_OBJECT_GC_MARK;

#ifdef NBASE_DEBUG_GC
    printf(NBASE_PRINT_YELLOW"nbase_gc: marked= %p\n"NBASE_PRINT_DEFAULT, (void*)pObj);
    g_state.gc_marked++;
#endif /* NBASE_DEBUG_GC */
}

/* ******************************************************************************** */

void nbase_gc_mark_all()
{
    /* Mark variables in use through vars area descriptors. */
    uint64_t* p = (uint64_t*)NBASE_VARS_AREA_BASE;
    while(p < g_state.vars_limit)
    {
        if(*p & NBASE_OBJECTBIT_MASK)
        {
            /*printf("DATA OFFSET: %d\n", NBASE_VARDATA_OFFSET(*p));*/
            NBASE_OBJECT* o = *(NBASE_OBJECT**)(NBASE_DATA_AREA_BASE + NBASE_VARDATA_OFFSET(*p));
            nbase_gc_mark(o);
        }

        p++;
    }
}

/* ******************************************************************************** */

void nbase_gc_sweep()
{
    NBASE_OBJECT** o = &g_state.obj_list;
    while(*o)
    {
        if(!((*o)->obj_type & NBASE_OBJECT_GC_MARK))
        {
            /* Unreached object, remove from list of objects and free it. */
            NBASE_OBJECT* unreached = *o;
            *o = unreached->next;
#ifdef NBASE_DEBUG_GC
            g_state.gc_destroyed++;
#endif /* NBASE_DEBUG_GC */
            nbase_destroy_object(unreached);
        }
        else
        {
            /* Object reached, unmark it! */
            (*o)->obj_type &= ~NBASE_OBJECT_GC_MARK;
            o = &(*o)->next;
        }
    }
}

/* ******************************************************************************** */

void nbase_gc()
{
#ifdef NBASE_DEBUG_GC
    g_state.gc_marked = 0;
    g_state.gc_destroyed = 0;
    int32_t num_objs = g_state.num_objs;
#endif /* NBASE_DEBUG_GC */

    nbase_gc_mark_all();
#ifdef NBASE_DEBUG_GC
    printf(NBASE_PRINT_YELLOW"nbase_gc: num_objs= %u\n"NBASE_PRINT_DEFAULT, num_objs);
    printf(NBASE_PRINT_YELLOW"nbase_gc: live= %u\n"NBASE_PRINT_DEFAULT, g_state.gc_marked);
#endif /* NBASE_DEBUG_GC */

    nbase_gc_sweep();
#ifdef NBASE_DEBUG_GC
    printf(NBASE_PRINT_YELLOW"nbase_gc: destroyed= %u\n"NBASE_PRINT_DEFAULT, g_state.gc_destroyed);
#endif /* NBASE_DEBUG_GC */

    /* Update max_objs, this makes the heap grow or shrink depending on last
        count of live objects. */
    g_state.max_objs = g_state.num_objs * 2;
}

/* ******************************************************************************** */

void nbase_print_info_and_version()
{
    printf("\n");
    printf("**** NBASE v%d.%d.%d - (C) 2022 Pablo A. Arrobbio, All Rights Reserved ****\n",
        NBASE_VERSION_MAYOR, NBASE_VERSION_MINOR, NBASE_VERSION_PATCH);
    printf("\n");
}

/* ******************************************************************************** */

void nbase_prompt()
{
    printf("\nREADY.\n");
}

/* ******************************************************************************** */

/*!
 * Creates a new AST node.
 *  @param pType Type of node.
 *  @param pDataType Type of data carried in the node.
 *  @param pStrVal String value if node is a string.
 *  @param pIntVal Integer number value if node is an integer value.
 *  @param pLhs Left side of binary expression if node is a binary expression.
 *  @param pRhs Right side of binary expression if node is a binary expression.
 *  @param pOper Operator of a binary expression.
 *  @param pExtra Extra data in node.
 */
NBASE_OBJECT* nbase_alloc_ast_node(nbase_ast_type pType, nbase_datatype pDataType,
    const char* pStrVal, NBASE_INTEGER pIntVal, NBASE_FLOAT pFltVal,
    NBASE_OBJECT* pLhs, NBASE_OBJECT* pRhs,
    uint16_t pOper, void* pExtra)
{
    nbase_ast_node* node;

    if(g_state.num_objs > g_state.max_objs)
        g_state.state_flags |= nbase_state_flag_GC_PENDING;
    
    NBASE_ASSERT((node = (nbase_ast_node*)nbase_malloc(sizeof(nbase_ast_node))));
    
    node->obj_type = nbase_object_type_AST_NODE;
    node->ast_type = pType;
    node->data_type = pDataType;

    switch(pType)
    {
    case nbase_ast_type_FACTOR:
        switch(pDataType)
        {
        case nbase_datatype_FLOAT:
            node->u.flt_val = pFltVal;
            break;
        
        case nbase_datatype_INTEGER:
            node->u.int_val = pIntVal;
            break;

        case nbase_datatype_STRING:
            if(!pExtra)
            {
                ((char*)pStrVal)[strlen(pStrVal) - 1] = '\0';
                NBASE_ASSERT((node->u.str_val = nbase_malloc(strlen(pStrVal) + 1)));
                strcpy(node->u.str_val, pStrVal);
            }
            else
            {
                /* If we have 'pExtra', it's not a literal string. */
            }
            break;
        
        default:
            NBASE_ASSERT_OR_INTERNAL_ERROR(0,
                "UNKNOWN DATA TYPE FOR AST NODE", THIS_FILE, __FUNCTION__, __LINE__);
        }
        break;

    case nbase_ast_type_BINARY:
        node->u.op.lhs = pLhs;
        node->u.op.rhs = pRhs;
        node->u.op.oper = pOper;
        break;

    case nbase_ast_type_UNARY:
        node->u.op.lhs = pLhs;
        node->u.op.oper = pOper;
        break;
    
    case nbase_ast_type_VARIABLE:
        NBASE_ASSERT((node->extra = (nbase_variable*)nbase_malloc(sizeof(nbase_variable))));
        memcpy(node->extra, pExtra, sizeof(nbase_variable));
        break;
    
    case nbase_ast_type_COMMAND_PRINT:
        node->u.op.lhs = pLhs;
        break;
    
    default:
        NBASE_ASSERT_OR_INTERNAL_ERROR(0,
            "UNKNOWN AST NODE", THIS_FILE, __FUNCTION__, __LINE__);
    }

    /* Add object to objects list. */
    node->next = g_state.obj_list;
    g_state.obj_list = (NBASE_OBJECT*)node;
    /* Count object. */
    g_state.num_objs++;

    return (NBASE_OBJECT*)node;
}

/* ******************************************************************************** */

/*!
 * Destroys an AST node.
 */
void nbase_destroy_ast_node(NBASE_OBJECT* pNode)
{
    nbase_ast_node* ast_node = (nbase_ast_node*)pNode;
    switch(ast_node->ast_type)
    {
    case nbase_ast_type_FACTOR:
        switch(ast_node->data_type)
        {
        case nbase_datatype_FLOAT:
            break;
        
        case nbase_datatype_INTEGER:
            break;
        
        case nbase_datatype_STRING:
            nbase_free(ast_node->u.str_val);
            break;
        
        default:
            NBASE_ASSERT_OR_INTERNAL_ERROR(0,
                "UNKNOWN DATA TYPE FOR AST NODE", THIS_FILE, __FUNCTION__, __LINE__);
        }
        break;
    
    case nbase_ast_type_BINARY:
    case nbase_ast_type_UNARY:
    case nbase_ast_type_COMMAND_PRINT:
        break;

    case nbase_ast_type_VARIABLE:
        nbase_free(ast_node->extra);
        break;
    
    default:
        NBASE_ASSERT_OR_INTERNAL_ERROR(0,
            "UNKNOWN AST NODE", THIS_FILE, __FUNCTION__, __LINE__);
    }

    /* Free this object here. */
    nbase_free(pNode);
}

/* ******************************************************************************** */

void nbase_error(nbase_error_type pErrType, const char* pSrcFile, const char* pSrcFunc,
    uint32_t pSrcLine, const char* pMsg, ...)
{
    va_list vargs;

    va_start(vargs, pMsg);

    NBASE_PRINTF("ERROR (%s:%s:%d)[%d]: %s",
        pSrcFile, pSrcFunc, pSrcLine, g_state.in_line + 1, g_nbase_error_messages[pErrType - 1]);
    vprintf(pMsg, vargs);

    va_end(vargs);

#ifndef NBASE_ENABLE_LONGJMP_ERROR_RECOVERY
    NBASE_PRINT("\nInterpreter terminated.\n");
    exit(-pErrType);
#else
    NBASE_THROW(pErrType);
#endif /* NBASE_ENABLE_LONGJMP_ERROR_RECOVERY */
}

/* ******************************************************************************** */

void nbase_internal_error(const char *pMsg,
    const char* pSrcFile, const char* pSrcFunc, uint32_t pSrcLine, ...)
{
    va_list vargs;

    va_start(vargs, pSrcLine);

    NBASE_PRINTF("INTERNAL ERROR (%s:%s:%d)[%d]: ",
        pSrcFile, pSrcFunc, pSrcLine, g_state.in_line + 1);
    vprintf(pMsg, vargs);
    NBASE_PRINT("\nInterpreter terminated.\n");

    va_end(vargs);
    exit(-1);
}

int32_t nbase_get_next_token(NBASE_BOOL pInterpret, NBASE_BOOL pVariableName)
{

}

void nbase_reset_state()
{
    int32_t num_objs;
    
    g_state.nextchar = NULL;
    g_state.in_line = 0;
    g_state.n_lines = 0;
    g_state.int32val = 0;
    g_state.paren_level = 0;
    g_state.state_flags &= ~nbase_state_flag_TOKENIZING;
    
    /* Clear variables and data area. */
    memset(NBASE_VARS_AREA_BASE, 0, NBASE_MAX_VARS_AREA_SIZE);
    memset(NBASE_DATA_AREA_BASE, 0, NBASE_MAX_DATA_AREA_SIZE);
    memset(NBASE_CODE_AREA_BASE, 0, NBASE_MAX_RAM_SIZE - NBASE_MAX_VARS_AREA_SIZE - NBASE_MAX_DATA_AREA_SIZE);
    g_state.vars_limit = (uint64_t*)NBASE_VARS_AREA_BASE;
    g_state.data_limit = NBASE_DATA_AREA_BASE;
    g_state.code_limit = NBASE_CODE_AREA_BASE;

    /* Perform garbage collection. */
    
    g_state.gc_marked = 0;
    g_state.gc_destroyed = 0;
    num_objs = g_state.num_objs;
    nbase_gc_mark_all();
/*#define DBG*/
#ifdef DBG
    printf("\nGC: num_objs= %u\n", num_objs);
    printf("    live= %u\n", g_state.gc_marked);
#endif /* DBG */
    nbase_gc_sweep();
#ifdef DBG
    printf("    destroyed= %u\n", g_state.gc_destroyed);
#endif /* DBG */
    g_state.max_objs = num_objs * 2;
#ifdef DBG
    printf("    final max_objs= %u\n", g_state.max_objs);
#endif /* DBG */
}

#endif /* NBASE_IMPLEMENTATION */

/* -------------------------------------------------------------------------------- */

void nbase_atexit()
{
    nbase_free(g_state.mem_pool);
}

int main(int argc, char** argv)
{
    atexit(nbase_atexit);

    g_state.nextchar = NULL;
    g_state.in_line = 0;
    g_state.n_lines = 0;
    g_state.int32val = 0;
    g_state.paren_level = 0;
    g_state.num_objs = 0;
    g_state.max_objs = NBASE_GC_INITIAL_THRESHOLD;
    g_state.obj_list = NULL;
    g_state.mem_pool = nbase_malloc(NBASE_MAX_RAM_SIZE);
    memset(g_state.mem_pool, 0, NBASE_MAX_RAM_SIZE);
    g_state.vars_limit = (uint64_t*)NBASE_VARS_AREA_BASE;
    g_state.data_limit = NBASE_DATA_AREA_BASE;
    g_state.code_limit = NBASE_CODE_AREA_BASE;

    nbase_print_info_and_version();

    /* REPL */
    while(true)
    {
        NBASE_TRY
        {
            nbase_prompt();
            fgets(g_state.input_buffer, NBASE_MAX_LINE_LEN_PLUS_1, stdin);
            
            g_state.nextchar = g_state.input_buffer;
cont2:        
            g_state.next_tok = nbase_get_next_token(true, false);
            if(g_state.next_tok == nbase_token_EOL)
                g_state.in_line++;
            else if(g_state.next_tok == nbase_token_COLON)
                goto cont2;
        
            if(g_state.state_flags & nbase_state_flag_GC_PENDING)
            {
                g_state.state_flags &= ~nbase_state_flag_GC_PENDING;
                nbase_gc();
            }
        }
        NBASE_CATCH
        {
            nbase_reset_state();
        }
        NBASE_ENDTRY;
    }
}
