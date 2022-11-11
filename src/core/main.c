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
#define NBASE_DEBUG_GC
/* Output tokenizer's debugging info to standard console output. */ 
/*#define NBASE_DEBUG_TOKENIZER*/
/* Enable color escape sequences to standard console output. */
#define NBASE_USE_ESCAPE_ANSI_COLORS

/* Enable setjmp/longjmp error recovering. */
#define NBASE_ENABLE_LONGJMP_ERROR_RECOVERY

#ifdef NBASE_USE_ESCAPE_ANSI_COLORS
#define NBASE_PRINT_CYAN       "\x1b[36m"
#define NBASE_PRINT_MAGENTA    "\x1b[35m"
#define NBASE_PRINT_GREEN      "\x1b[32m"
#define NBASE_PRINT_YELLOW     "\x1b[33m"
#define NBASE_PRINT_RED        "\x1b[31m"
#define NBASE_PRINT_DEFAULT    "\x1b[0m"
#else
#define NBASE_PRINT_CYAN       ""
#define NBASE_PRINT_MAGENTA    ""
#define NBASE_PRINT_GREEN      ""
#define NBASE_PRINT_YELLOW     ""
#define NBASE_PRINT_RED        ""
#define NBASE_PRINT_DEFAULT    ""
#endif /* NBASE_USE_ESCAPE_ANSI_COLORS */

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
#define NBASE_VERSION_MINOR 1
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

#define NBASE_ASSERT_OPERAND_TO_UNARY(type, types_comp, oper, func, line, expected) \
    if(!( types_comp )) { \
        nbase_error(nbase_error_type_INVALID_OPERANDS_UNARY, THIS_FILE, func, line, " %s, expected: %s, got: %s", \
        nbase_get_oper_name(oper), expected, g_nbase_type_names[type]); \
    }

#define NBASE_ASSERT_OPERAND_TO_BINARY(node1, node2, types_comp, oper, func, line, expected) \
    if((node1 ->data_type != node2 ->data_type) || !( types_comp )) { \
        nbase_error(nbase_error_type_INVALID_OPERANDS_BINARY, THIS_FILE, func, line, " %s, expected both: %s, got: %s and %s", \
        nbase_get_oper_name(oper), expected, g_nbase_type_names[node1 ->data_type], g_nbase_type_names[node2 ->data_type]); \
    }

#define NBASE_AST_NODE(x)               ((nbase_ast_node*)x)

#define NBASE_DECLARE_OBJECT(struct_name) \
    typedef struct _ ## struct_name \
    { \
        uint8_t obj_type; \
        NBASE_OBJECT* next; \

#define NBASE_END_OBJECT(struct_name) \
    } struct_name;

#define NBASE_TOKENIZING (g_state.state_flags & nbase_state_flag_TOKENIZING)

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
    nbase_token_NOT, nbase_token_GC, nbase_token_END, /*nbase_token_CLEAR,*/
    nbase_token_LIST, /*nbase_token_GOTO,*/ nbase_token_LOADLINE, nbase_token_RUN,
    nbase_token_PRINT_ALONE,
#ifdef NBASE_INCLUDE_FEATURE_DEBUGTOOLS
    nbase_token_LVAR, nbase_token_STAT, nbase_token_DUMP,
#endif /* NBASE_INCLUDE_FEATURE_DEBUGTOOLS */

    nbase_token_NL,
    nbase_token_EOL,
    nbase_token_LAST
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
    nbase_error_type_UNRECOGNIZED_LEXER_TOKEN, /*nbase_error_type_CANT_OPEN_INPUT_FILE,*/
    nbase_error_type_DIVISION_BY_ZERO, nbase_error_type_DOM_ERROR_1,
    nbase_error_type_DOM_ERROR_2, nbase_error_type_unknown_TOKEN
} nbase_error_type;

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
void                nbase_print_ast_node(nbase_ast_node* pNode, const char* pNodeMsg);
void                nbase_error(nbase_error_type pErrType, const char* pSrcFile, const char* pSrcFunc,
                        uint32_t pSrcLine, const char* pMsg, ...);
void                nbase_internal_error(const char *pMsg,
                        const char* pSrcFile, const char* pSrcFunc, uint32_t pSrcLine, ...);
NBASE_BOOL          nbase_is_integer_expr(nbase_ast_node* pNode, int pSign);
void                nbase_assert_term_present(NBASE_OBJECT* pNode);
NBASE_OBJECT*       nbase_parse_factor(NBASE_BOOL* pForceNotNull);
NBASE_OBJECT*       nbase_parse_term(NBASE_BOOL* pForceNotNull);
NBASE_OBJECT*       nbase_parse_sum(NBASE_BOOL* pForceNotNull);
NBASE_OBJECT*       nbase_parse_logic_op(NBASE_BOOL* pForceNotNull);
NBASE_OBJECT*       nbase_parse_expression(NBASE_BOOL* pForceNotNull);
void                nbase_rtrim(char* pStr);
void                nbase_skip_spaces();
nbase_keyword*      nbase_search_keyword(const char* pToken);
nbase_token         nbase_search_keyword_token(const char* pToken);
void                nbase_parse_colon();
nbase_token         nbase_parse_number();
NBASE_BOOL          nbase_parse_identifier(NBASE_BOOL pVariableName);
int32_t             nbase_parse_single_char_token(char pTest, bool pUpdateParent);
int32_t             nbase_get_next_token(NBASE_BOOL pInterpret, NBASE_BOOL pVariableName);
uint32_t            nbase_get_size_of_type(nbase_datatype pType);
void                nbase_reset_state();

#define KEYWORDS_DEFINITIONS
#include "keywords.c"
#define TOKEN_DEFINITIONS
#include "token.c"
#define RUN_DEFINITIONS
#include "run.c"
#define PRINT_DEFINITIONS
#include "print.c"
#define EVAL_DEFINITIONS
#include "eval.c"
#define VARS_DEFINITIONS
#include "vars.c"
#define OPER_DEFINITIONS
#include "oper.c"

#ifdef NBASE_INCLUDE_FEATURE_DEBUGTOOLS
#define DEBUGTOOLS_LVAR_DEFINITIONS
#include "../debugtools/lvar.c"
#define DEBUGTOOLS_STAT_DEFINITIONS
#include "../debugtools/stat.c"
#define DEBUGTOOLS_DUMP_DEFINITIONS
#include "../debugtools/dump.c"
#endif /* NBASE_INCLUDE_FEATURE_DEBUGTOOLS */

/* -------------------------------------------------------------------------------- */

#define NBASE_IMPLEMENTATION

#ifdef NBASE_IMPLEMENTATION

/*! Global interpreter state. */
nbase_state g_state;
/*! Temporal buffer. */
char g_temp_buff[NBASE_MAX_LINE_LEN_PLUS_1];

/* -------------------------------------------------------------------------------- */

#define KEYWORDS_IMPLEMENTATION
#include "keywords.c"
#define TOKEN_IMPLEMENTATION
#include "token.c"
#define RUN_IMPLEMENTATION
#include "run.c"
#define PRINT_IMPLEMENTATION
#include "print.c"
#define EVAL_IMPLEMENTATION
#include "eval.c"
#define VARS_IMPLEMENTATION
#include "vars.c"
#define OPER_IMPLEMENTATION
#include "oper.c"

#ifdef NBASE_INCLUDE_FEATURE_DEBUGTOOLS
#define DEBUGTOOLS_LVAR_IMPLEMENTATION
#include "../debugtools/lvar.c"
#define DEBUGTOOLS_STAT_IMPLEMENTATION
#include "../debugtools/stat.c"
#define DEBUGTOOLS_DUMP_IMPLEMENTATION
#include "../debugtools/dump.c"
#endif /* NBASE_INCLUDE_FEATURE_DEBUGTOOLS */

/* -------------------------------------------------------------------------------- */

static const char* THIS_FILE = "main.c";

/*! Type names. */
char *g_nbase_type_names[] =
{
    "UNKNOWN",
    "FLOAT",
    "INTEGER",
    "STRING"
};

/*! Keyword table. */
nbase_keyword g_nbase_keywords[] =
{
    {"PRINT",           nbase_keyword_PRINT,    nbase_token_PRINT},
    {"REM",             /*nbase_keyword_REM*/NULL,      nbase_token_REM},
    {"DIM",             /*nbase_keyword_DIM*/NULL,      nbase_token_DIM},
    {"LET",             /*nbase_keyword_LET*/NULL,      nbase_token_LET},

    {"MOD",             NULL,                   nbase_token_MOD},

    {"LSHIFT",          NULL,                   nbase_token_LSHIFT},
    {"RSHIFT",          NULL,                   nbase_token_RSHIFT},
    {"<=",              NULL,                   nbase_token_LESSEQ},
    {">=",              NULL,                   nbase_token_GREATEREQ},
    {"<>",              NULL,                   nbase_token_NEQUALS},

    {"AND",             NULL,                   nbase_token_AND},
    {"OR",              NULL,                   nbase_token_OR},
    {"XOR",             NULL,                   nbase_token_XOR},
    {"NOT",             NULL,                   nbase_token_NOT},
    {"GC",              nbase_keyword_GC,       nbase_token_GC},
    {"END",             nbase_keyword_END,      nbase_token_END},
    /*{"CLEAR",           nbase_keyword_CLEAR,    nbase_token_CLEAR},*/
    {"LIST",            /*nbase_keyword_LIST*/NULL,     nbase_token_LIST},
    /*{"GOTO",            *nbase_keyword_GOTO,     nbase_token_GOTO},*/
#if 0
    {"IF#",             nbase_keyword_IF_NUM,   nbase_token_IF_NUM},
#endif
    {"RUN",             nbase_keyword_RUN,      nbase_token_RUN},
    
#ifdef NBASE_INCLUDE_FEATURE_DEBUGTOOLS
    {"LVAR",            nbase_keyword_LVAR,     nbase_token_LVAR},
    {"STAT",            nbase_keyword_STAT,     nbase_token_STAT},
    {"DUMP",            nbase_keyword_DUMP,     nbase_token_DUMP},
#endif /* NBASE_INCLUDE_FEATURE_DEBUGTOOLS */    
    
    {".",               nbase_keyword_LOADLINE, nbase_token_LOADLINE},
    
    {NULL,              NULL,                   0}
};

const char* g_nbase_error_messages[] = {
    "(PARSERR1) NUMBER PARSE ERROR",
    "(PARSERR2) UNKNOWN SYMBOL",
    "(PARSERR2.1) DUPLICATED SYMBOL",
    "(PARSERR3) UNRECOGNIZED TOKEN BY PARSER",
    "(PARSERR4) EXPECTED KEYWORD",
    "(PARSERR5) UNKNOWN KEYWORD",
    "(PARSERR6) INVALID OPERANDS TO UNARY",
    "(PARSERR7) INVALID OPERANDS TO BINARY",
    "(PARSERR8) PARENTHESES MISMATCH",
    "(PARSERR9) EXPECTED POSITIVE VALUE",
    "(PARSERR10) EXPECTED NEGATIVE VALUE",
    "(PARSERR11) EXPECTED ZERO",
    "(PARSERR12) EXPECTED EXPRESSION",
    "(PARSERR13) EXPECTED INTEGER EXPRESSION FOR ARRAY DIMENSION",
    "(PARSERR14) UNTERMINATED STRING",
    "(PARSERR15) RESERVED WORD USED FOR SYMBOL",
    "(PARSERR16) EXPECTED IDENTIFIER",
    "(PARSERR17) EXPECTED END OF LINE",
    "(PARSERR18) UNEXPECTED TOKEN",
    "(PARSERR19) EXPECTED '='",
    "(PARSERR19.1) EXPECTED 'THEN'",
    "(PARSERR20) INCOMPATIBLE TYPES IN ASSIGNMENT",
    "(LEXERR1) UNRECOGNIZED TOKEN BY LEXER",
    /*"(OTHERERR1) CAN'T OPEN INPUT FILE",*/
    "(MATHERR1) DIVISION BY ZERO",
    "(MATHERR2) DOM ERROR 1, FE_INVALID RAISED",
    "(MATHERR3) DOM ERROR 2, FE_DIVBYZERO RAISED",
    "(RUNERR1) UNKNOWN TOKEN"
};

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
 *  @param pFltVal Floating point number value if node is a floating point value.
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

static int32_t g_tab_level1 = 0;

/*!
 *  Prints an AST node for debugging purposes.
 */
void nbase_print_ast_node(nbase_ast_node* pNode, const char* pNodeMsg)
{
    int32_t i;
    for(i = 0; i < g_tab_level1; i++)
        NBASE_PRINT("  ");

    if(pNodeMsg)
        NBASE_PRINTF("%s ", pNodeMsg);

    if(!pNode)
        return;
    
    switch(pNode->ast_type)
    {
    case nbase_ast_type_FACTOR:
        NBASE_PRINT("FACTOR: data_type= ");
        switch(pNode->data_type)
        {
        case nbase_datatype_FLOAT:
            if(!pNode->extra)
            {
                NBASE_PRINTF("float, value=%.50f\n", pNode->u.flt_val);
            }
            else
            {
                NBASE_PRINTF("float, variable...\n");
            }
            break;

        case nbase_datatype_INTEGER:
            if(!pNode->extra)
            {
                NBASE_PRINTF("integer, value=%d\n", pNode->u.int_val);
            }
            else
            {
                NBASE_PRINT("integer, variable...\n");
            }
            break;

        case nbase_datatype_STRING:
            if(!pNode->extra)
            {
                NBASE_PRINTF("string, value=%s\n", pNode->u.str_val);
            }
            else
            {
                NBASE_PRINT("string, variable...\n");
            }
            break;
        
        default:
            NBASE_ASSERT_OR_INTERNAL_ERROR(0,
                "UNKNOWN DATA TYPE FOR AST NODE", THIS_FILE, __FUNCTION__, __LINE__);
        }
        break;
    
    case nbase_ast_type_BINARY:
        NBASE_PRINTF("BINARY: oper= %s \n", nbase_get_oper_name(pNode->u.op.oper));
        g_tab_level1++;
        if(pNode->u.op.lhs)
            nbase_print_ast_node((nbase_ast_node*)pNode->u.op.lhs, "LHS");
        if(pNode->u.op.rhs)
            nbase_print_ast_node((nbase_ast_node*)pNode->u.op.rhs, "RHS");
        g_tab_level1--;
        break;

    case nbase_ast_type_UNARY:
        NBASE_PRINTF("UNARY: oper= %s \n", nbase_get_oper_name(pNode->u.op.oper));
        g_tab_level1++;
        nbase_print_ast_node((nbase_ast_node*)pNode->u.op.lhs, "LHS");
        g_tab_level1--;
        break;
    
    case nbase_ast_type_VARIABLE:
    {
        nbase_variable* var = pNode->extra;
        uint32_t i;
        NBASE_PRINTF("VARIABLE: name=%s data_type= %s data_offset=%d dims=", var->name, g_nbase_type_names[var->type], var->data_offset);
        for(i = 0; i < NBASE_MAX_ARRAY_DIMENSION; i++)
            NBASE_PRINTF("%d ", var->dims[i]);
        NBASE_PRINT("\n");
    }
        break;
    
    default:
        NBASE_ASSERT_OR_INTERNAL_ERROR(0,
            "UNKNOWN AST NODE", THIS_FILE, __FUNCTION__, __LINE__);
    }
}

/* ******************************************************************************** */

void nbase_error(nbase_error_type pErrType, const char* pSrcFile, const char* pSrcFunc,
    uint32_t pSrcLine, const char* pMsg, ...)
{
    va_list vargs;

    va_start(vargs, pMsg);

#ifdef NBASE_DEBUG__
    NBASE_PRINTF("ERROR (%s: %s(): %d)[%d]: %s",
        pSrcFile, pSrcFunc, pSrcLine, g_state.in_line + 1, g_nbase_error_messages[pErrType - 1]);
#else
    NBASE_PRINTF("ERROR[%d]: %s",
        g_state.in_line + 1, g_nbase_error_messages[pErrType - 1]);
#endif /* NBASE_DEBUG */
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

    NBASE_PRINTF("INTERNAL ERROR (%s: %s(): %d)[%d]: ",
        pSrcFile, pSrcFunc, pSrcLine, g_state.in_line + 1);
    vprintf(pMsg, vargs);
    NBASE_PRINT("\nInterpreter terminated.\n");

    va_end(vargs);
    exit(-1);
}

/* ******************************************************************************** */

/*!
 * Indicates if node is an integer expression.
 */
NBASE_BOOL nbase_is_integer_expr(nbase_ast_node* pNode, int pSign)
{
    if(!pNode)
        return false;
    if (pNode->ast_type != nbase_ast_type_FACTOR)
        return false;
    else
    {
        if(pSign > 0)
            NBASE_ASSERT_OR_ERROR(pNode->u.int_val > 0,
                nbase_error_type_EXPECTED_POSITIVE_VAL, THIS_FILE, __FUNCTION__, __LINE__, "");
        if(pSign < 0)
            NBASE_ASSERT_OR_ERROR(pNode->u.int_val < 0,
                nbase_error_type_EXPECTED_NEGATIVE_VAL, THIS_FILE, __FUNCTION__, __LINE__, "");
        if(pSign == 0)
            NBASE_ASSERT_OR_ERROR(pNode->u.int_val == 0,
                nbase_error_type_EXPECTED_ZERO_VAL, THIS_FILE, __FUNCTION__, __LINE__, "");
    }
    if (pNode->data_type != nbase_datatype_INTEGER)
        return false;
    /*if (pNode->extra)
        return false;*/

    return true;
}

/* ******************************************************************************** */

/*!
 * Used to check if a following expression is present.
 */
void nbase_assert_term_present(NBASE_OBJECT* pNode)
{
    NBASE_ASSERT_OR_ERROR(pNode,
        nbase_error_type_EXPECTED_EXPRESSION, THIS_FILE, __FUNCTION__, __LINE__, "");
}

/* ******************************************************************************** */

/*!
 * Parse a factor.
 */
NBASE_OBJECT* nbase_parse_factor(NBASE_BOOL* pForceNotNull)
{
    nbase_variable var;
    NBASE_OBJECT* node = NULL;

    nbase_skip_spaces();
    
    /* Is factor a number? */
    if (isdigit(*g_state.nextchar))
    {
        g_state.next_tok = nbase_get_next_token(false, false);
        if (g_state.next_tok != nbase_token_LEXER_ERROR)
        {
            if(g_state.next_tok == nbase_token_FLOAT_LITERAL)
                node = nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_FLOAT,
                    NULL, 0, g_state.fltval, NULL, NULL, 0, NULL);
            else if(g_state.next_tok == nbase_token_INTEGER_LITERAL)
                node = nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_INTEGER,
                    NULL, g_state.int32val, 0, NULL, NULL, 0, NULL);
            
            nbase_tokenize_factor((nbase_ast_node*)node);
        }
        else
            nbase_error(nbase_error_type_NUMBER_ERROR, THIS_FILE, __FUNCTION__, __LINE__, "");
    }
    /* Read a '(' ? */
    else if (*g_state.nextchar == nbase_token_LEFTPAREN)
    {
        nbase_tokenize_keyword(nbase_token_LEFTPAREN);
        
        g_state.paren_level++;
        g_state.nextchar++;
        *pForceNotNull = true;
        node = nbase_parse_expression(pForceNotNull);
        NBASE_ASSERT_OR_ERROR(g_state.next_tok == nbase_token_RIGHTPAREN,
            nbase_error_type_PARENTHESES_MISMATCH, THIS_FILE, __FUNCTION__, __LINE__, "");

        nbase_tokenize_keyword(nbase_token_RIGHTPAREN);
    }
    /* Is factor a string literal? */
    else if (*g_state.nextchar == '"')
    {
        g_state.next_tok = nbase_get_next_token(false, false);
    
        node = nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_STRING,
            g_state.token + 1, 0, 0, NULL, NULL, 0, NULL);

        nbase_tokenize_factor((nbase_ast_node*)node);
    }
    /* Read a ':' ? */
    else if (*g_state.nextchar == ':')
    {
        g_state.next_tok = nbase_get_next_token(false, false);
        return NULL;
    }
    /* Variable? */
    else if (isalpha(*g_state.nextchar))
    {
        nbase_parse_identifier(true);
        nbase_search_variable(g_state.token, &var);
        if(var.type != nbase_datatype_NONE)
        {
            nbase_tokenize_var(var.type, var.name);
            
            /* Check for array dimension. */
            nbase_skip_spaces();
            if(*g_state.nextchar == nbase_token_LEFTPAREN)
            {
                uint32_t dimi = 0;
                NBASE_DIMENSION dims[NBASE_MAX_ARRAY_DIMENSION] = {0};
                NBASE_BOOL force_not_null;
                
                nbase_tokenize_keyword(nbase_token_LEFTPAREN);

                g_state.next_tok = nbase_get_next_token(false, false);
                g_state.paren_level++;

                force_not_null = true;
                do
                {
                    nbase_ast_node* node = (nbase_ast_node*)nbase_parse_expression(&force_not_null);
                    /*node = (nbase_ast_node*)nbase_eval_expression((NBASE_OBJECT*)node);*/
                    NBASE_ASSERT_OR_ERROR(nbase_is_integer_expr(node, 1),
                        nbase_error_type_DIM_NOT_INT, THIS_FILE, __FUNCTION__, __LINE__, "");

                    dims[dimi++] = (int)node->u.int_val;
                    if(dimi >= NBASE_MAX_ARRAY_DIMENSION)
                        break;
                
                    if(g_state.next_tok == nbase_token_COMMA)
                        nbase_tokenize_keyword(nbase_token_COMMA);
                
                } while(g_state.next_tok == nbase_token_COMMA);

                NBASE_ASSERT_OR_ERROR(g_state.next_tok == nbase_token_RIGHTPAREN,
                    nbase_error_type_PARENTHESES_MISMATCH, THIS_FILE, __FUNCTION__, __LINE__, "");
            
                nbase_tokenize_keyword(nbase_token_RIGHTPAREN);

                memcpy(var.dims, dims, NBASE_MAX_ARRAY_DIMENSION * sizeof(NBASE_DIMENSION));
            }

            node = nbase_alloc_ast_node(nbase_ast_type_VARIABLE, var.type,
                NULL, 0, 0, NULL, NULL, 0, &var);
        }
        else
            nbase_error(nbase_error_type_UNKNOWN_SYMBOL,
                THIS_FILE, __FUNCTION__, __LINE__, ": %s", g_state.token);
    }
    else
    {
        /* TODO(Pablo): check this for regression. */
        /* Added to forbid two consecutive operators..., recheck cases!!! maybe unnecessary!!! */
        if(*g_state.nextchar && !isspace(*g_state.nextchar))
            *pForceNotNull = true;
        
        if(*pForceNotNull)
            nbase_error(nbase_error_type_UNRECOGNIZED_PARSER_TOKEN, THIS_FILE, __FUNCTION__, __LINE__,
                " at: ... %s", g_state.nextchar);
        
        /* End of line reached. */
        if(*g_state.nextchar == '\n' || *g_state.nextchar == '\0')
            return NULL;
    }

    g_state.next_tok = nbase_get_next_token(false, false);

    return node;
}

/* ******************************************************************************** */

/*!
 * Parse a term.
 */
NBASE_OBJECT* nbase_parse_term(NBASE_BOOL* pForceNotNull)
{
    nbase_token unary = nbase_token_NONE;
    NBASE_OBJECT* node = NULL, *node2 = NULL;
    uint16_t oper;
    
    /* Take into account unary operators (%, NOT, etc.), necessarily skip spaces. */
    nbase_skip_spaces();
    if(*g_state.nextchar == nbase_token_NEGATED ||       /* %   */
        !strncmp(g_state.nextchar, "NOT", 3)             /* NOT */
    )  
    {
        g_state.next_tok = unary = nbase_get_next_token(false, false);
        /* If unary operator found, ensure an expression is present. */
        *pForceNotNull = true;
    }
    else {
        node = nbase_parse_factor(pForceNotNull);
        if(*pForceNotNull)
            nbase_assert_term_present(node);
    }

    /* Loop while a valid operation inside a term is found... */
    while (g_state.next_tok == nbase_token_MUL ||       /* MUL */
        g_state.next_tok == nbase_token_DIV ||          /* DIV */
        g_state.next_tok == nbase_token_MOD ||          /* MOD */
        g_state.next_tok == nbase_token_NEGATED ||      /* %   */
        g_state.next_tok == nbase_token_POW ||          /* ^   */
        g_state.next_tok == nbase_token_NOT             /* NOT */
    )
    {
        nbase_tokenize_keyword(g_state.next_tok);
        
        *pForceNotNull = true;
        oper = g_state.next_tok;
        node2 = nbase_parse_factor(pForceNotNull);
        if(*pForceNotNull)
            nbase_assert_term_present(node2);

        /* Check for compatible types in unary operation. */
        if(unary != nbase_token_NONE)
        {
            switch(unary)
            {
            case nbase_token_NOT:
            case nbase_token_NEGATED:
                NBASE_ASSERT_OPERAND_TO_UNARY(NBASE_AST_NODE(node2)->data_type,
                    NBASE_AST_NODE(node2)->data_type == nbase_datatype_INTEGER ||
                    NBASE_AST_NODE(node2)->data_type == nbase_datatype_FLOAT,
                    unary, __FUNCTION__, __LINE__, "INTEGER or FLOAT");
                break;

            default:
                NBASE_ASSERT_OR_INTERNAL_ERROR(0,
                    "UNKNOWN UNARY OPERATOR", THIS_FILE, __FUNCTION__, __LINE__);
            }
        }

        /* Check types of lhs & rhs for the binary operation. */
        if(oper == nbase_token_MOD)
        {
            NBASE_ASSERT_OPERAND_TO_BINARY(NBASE_AST_NODE(node), NBASE_AST_NODE(node2),
                NBASE_AST_NODE(node)->data_type == nbase_datatype_INTEGER,
                oper, __FUNCTION__, __LINE__, "INTEGER");
        }
        if(oper == nbase_token_MUL || oper == nbase_token_DIV || oper == nbase_token_POW)
        {
            NBASE_ASSERT_OPERAND_TO_BINARY(NBASE_AST_NODE(node), NBASE_AST_NODE(node2),
                NBASE_AST_NODE(node)->data_type == nbase_datatype_INTEGER ||
                NBASE_AST_NODE(node)->data_type == nbase_datatype_FLOAT,
                oper, __FUNCTION__, __LINE__, "INTEGER or FLOAT");
        }
    
        if(unary == nbase_token_NONE)
        {
            node = nbase_alloc_ast_node(nbase_ast_type_BINARY,
                NBASE_AST_NODE(node)->data_type, NULL, 0, 0.0f, node, node2, oper, NULL);
        }
        else
        {
            node = nbase_alloc_ast_node(nbase_ast_type_UNARY,
                NBASE_AST_NODE(node2)->data_type, NULL, 0, 0.0f, node2, NULL, unary, NULL);
            unary = nbase_token_NONE;
        }
    }
    
    return node;
}

/* ******************************************************************************** */

/*!
 * Parse a sum.
 */
NBASE_OBJECT* nbase_parse_sum(NBASE_BOOL* pForceNotNull)
{
    NBASE_OBJECT* node = NULL, *node2 = NULL;
    uint16_t oper;
    
    node = nbase_parse_term(pForceNotNull);
    if(*pForceNotNull)
        nbase_assert_term_present(node);

    /* Loop while a valid operation inside a sum is found... */
    while (g_state.next_tok == nbase_token_PLUS ||      /* +      */
        g_state.next_tok == nbase_token_MINUS ||        /* -      */
        g_state.next_tok == nbase_token_LSHIFT ||       /* LSHIFT */
        g_state.next_tok == nbase_token_RSHIFT          /* RSHIFT */
    )
    {
        nbase_tokenize_keyword(g_state.next_tok);

        *pForceNotNull = true;
        oper = g_state.next_tok;
        node2 = nbase_parse_term(pForceNotNull);
        if(*pForceNotNull)
            nbase_assert_term_present(node2);

        /* Check types of lhs & rhs for the binary operation. */
        if(oper == nbase_token_PLUS || oper == nbase_token_MINUS)
        {
            NBASE_ASSERT_OPERAND_TO_BINARY(NBASE_AST_NODE(node), NBASE_AST_NODE(node2),
                NBASE_AST_NODE(node)->data_type == nbase_datatype_INTEGER ||
                NBASE_AST_NODE(node)->data_type == nbase_datatype_FLOAT,
                oper, __FUNCTION__, __LINE__, "INTEGER or FLOAT");
        }
        if(oper == nbase_token_LSHIFT || oper == nbase_token_RSHIFT)
        {
            NBASE_ASSERT_OPERAND_TO_BINARY(NBASE_AST_NODE(node), NBASE_AST_NODE(node2),
                NBASE_AST_NODE(node)->data_type == nbase_datatype_INTEGER,
                oper, __FUNCTION__, __LINE__, "INTEGER");
        }
    
        node = nbase_alloc_ast_node(nbase_ast_type_BINARY,
            NBASE_AST_NODE(node)->data_type, NULL, 0, 0.0f, node, node2, oper, NULL);
    }
    
    return node;
}

/* ******************************************************************************** */

/*!
 * Parse a logic operation.
 */
NBASE_OBJECT* nbase_parse_logic_op(NBASE_BOOL* pForceNotNull)
{
    NBASE_OBJECT* node = NULL, *node2 = NULL;
    uint16_t oper;
    
    node = nbase_parse_sum(pForceNotNull);
    if(*pForceNotNull)
        nbase_assert_term_present(node);

    /* Loop while a valid operation inside a logic op is found... */
    while (g_state.next_tok == nbase_token_AND ||       /* AND */
        g_state.next_tok == nbase_token_OR ||           /* OR  */
        g_state.next_tok == nbase_token_XOR             /* XOR */
    )
    {
        nbase_tokenize_keyword(g_state.next_tok);

        *pForceNotNull = true;
        oper = g_state.next_tok;
        node2 = nbase_parse_sum(pForceNotNull);
        if(*pForceNotNull)
            nbase_assert_term_present(node2);

        /* Check types of lhs & rhs for the binary operation. */
        NBASE_ASSERT_OPERAND_TO_BINARY(NBASE_AST_NODE(node), NBASE_AST_NODE(node2),
            NBASE_AST_NODE(node)->data_type == nbase_datatype_INTEGER,
            oper, __FUNCTION__, __LINE__, "INTEGER");
    
        node = nbase_alloc_ast_node(nbase_ast_type_BINARY,
            NBASE_AST_NODE(node)->data_type, NULL, 0, 0.0f, node, node2, oper, NULL);
    }
    
    return node;
}

/* ******************************************************************************** */

/*!
 * Parse a toplevel expression.
 */
NBASE_OBJECT* nbase_parse_expression(NBASE_BOOL* pForceNotNull)
{
    NBASE_OBJECT* node = NULL, *node2 = NULL;
    uint16_t oper;
    
    node = nbase_parse_logic_op(pForceNotNull);
    if(*pForceNotNull)
        nbase_assert_term_present(node);

    /* Loop while a valid operation inside an expression is found... */
    while (g_state.next_tok == nbase_token_LESS ||      /* <  */
        g_state.next_tok == nbase_token_LESSEQ ||       /* <= */
        g_state.next_tok == nbase_token_GREATER ||      /* >  */
        g_state.next_tok == nbase_token_GREATEREQ ||    /* >= */
        g_state.next_tok == nbase_token_EQ ||           /* =  */
        g_state.next_tok == nbase_token_NEQUALS         /* <> */
    )
    {
        nbase_tokenize_keyword(g_state.next_tok);

        *pForceNotNull = true;
        oper = g_state.next_tok;
        node2 = nbase_parse_logic_op(pForceNotNull);
        if(*pForceNotNull)
            nbase_assert_term_present(node2);

        /* Check types of lhs & rhs for the binary operation. */
        NBASE_ASSERT_OPERAND_TO_BINARY(NBASE_AST_NODE(node), NBASE_AST_NODE(node2),
            NBASE_AST_NODE(node)->data_type == nbase_datatype_INTEGER ||
            NBASE_AST_NODE(node)->data_type == nbase_datatype_FLOAT,
            oper, __FUNCTION__, __LINE__, "INTEGER or FLOAT");
    
        node = nbase_alloc_ast_node(nbase_ast_type_BINARY,
            NBASE_AST_NODE(node)->data_type, NULL, 0, 0.0f, node, node2, oper, NULL);
    }
    
    return node;
}

/* ******************************************************************************** */

void nbase_rtrim(char* pStr)
{
    char* p = pStr + strlen(pStr) - 1;
    while(isspace(*p))
    {
        *p = '\0';
        p--;
    }
}

/* ******************************************************************************** */

void nbase_skip_spaces()
{
    while (*g_state.nextchar && *g_state.nextchar == ' ')
        g_state.nextchar++;
}

/* ******************************************************************************** */

nbase_keyword* nbase_search_keyword(const char* pToken)
{
    nbase_keyword* kw = g_nbase_keywords;
    while (kw->keyword)
    {
        if (!strcmp(kw->keyword, pToken))
            return kw;
        kw++;
    }

    return NULL;
}

/* ******************************************************************************** */

nbase_token nbase_search_keyword_token(const char* pToken)
{
    nbase_keyword* kw = nbase_search_keyword(pToken);
    if(kw)
        return kw->token;

    return nbase_token_NONE;
}

/* ******************************************************************************** */

void nbase_parse_colon()
{
    g_state.next_tok = nbase_get_next_token(false, false);
    if(g_state.next_tok != nbase_token_COLON)
    {
        NBASE_ASSERT_OR_ERROR(g_state.next_tok == nbase_token_EOL,
            nbase_error_type_EXPECTED_END_OF_LINE, KEYWORDS_FILE, __FUNCTION__, __LINE__, "");
    }
}

/* ******************************************************************************** */

NBASE_FLOAT ten(NBASE_INTEGER pE)
{
    NBASE_FLOAT x, t;

    x = 1.0f;
    t = 10.0f;
    while (pE > 0)
    {
        if (pE % 2)
        {
            x = t * x;
        }
        t = t * t;
        pE = pE / 2;
    }
    return x;
}

/* ******************************************************************************** */

/* NOTE(Pablo): number parsing method and routine taken
    from official Oberon 07 compiler. */
#define NUM_ERROR(msg) printf(msg "\n");
#define MAX_EXPONENT 38

nbase_token nbase_parse_number()
{
    char c = 0;
    const NBASE_INTEGER max = 2147483647; /* 2^31 - 1 */

    NBASE_INTEGER i, k, e, n, s, h;
    NBASE_FLOAT x;
    NBASE_INTEGER d[16];
    NBASE_BOOL negE;

    g_state.int32val = 0, g_state.fltval = 0, i = 0, n = 0, k = 0;
    c = *g_state.nextchar++;
    do
    {
        if (n < 16)
        {
            d[n] = c - 0x30;
            n++;
        }
        else
        {
            NUM_ERROR("Too many digits for number");
            n = 0;
            return nbase_token_LEXER_ERROR;
        }
        c = *g_state.nextchar++;
    } while ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'));
    
    if(c != '.' && c != 'H' && c != 'R' && c != 'X')
        g_state.nextchar--;

    if (c == 'H' || c == 'R' || c == 'X') /* Hex. */
    {
        do
        {
            h = d[i];
            if (h >= 10)
            {
                h = h - 7;
            }
            k = k * 0x10 + h;
            i++; /* No overflow check. */
        } while (i != n);

        if (c == 'X') /* Hex literal sufix. */
        {
            if (k < 0x100)
            {
                g_state.int32val = k;
                return nbase_token_INTEGER_LITERAL;
            }
            else
            {
                NUM_ERROR("Illegal value for 'X' prefix, expected hexadecimal value < 100H");
                g_state.int32val = 0;
                return nbase_token_LEXER_ERROR;
            }
        }
        else if (c == 'R') /* Real literal sufix. */
        {
            g_state.fltval = k;
            return nbase_token_FLOAT_LITERAL;
        }
        else
        {
            g_state.int32val = k;
            return nbase_token_INTEGER_LITERAL;
        }
    }
    else if (c == '.')
    {
        c = *g_state.nextchar++;
        /* Real number. */
        {
            x = 0.0f;
            e = 0;
            do
            { /* Integer part */
                x = x * 10.0f + (float)d[i];
                i++;
            } while (i != n);
            while (c >= '0' && c <= '9')
            { /* Fraction. */
                x = x * 10.0f + (float)(c - 0x30);
                e--;
                c = *g_state.nextchar++;
            }
            
            if(c != 'E' && c != 'D')
                g_state.nextchar--;

            if (c == 'E' || c == 'D')
            { /* Scale factor. */
                c = *g_state.nextchar++;
                s = 0;
                if (c == '-')
                {
                    negE = true;
                    c = *g_state.nextchar++;
                }
                else
                {
                    negE = false;
                    if (c == '+')
                    {
                        c = *g_state.nextchar++;
                    }
                }
                if (c >= '0' && c <= '9')
                {
                    do
                    {
                        s = s * 10 + c - 0x30;
                        c = *g_state.nextchar++;
                    } while (c >= '0' && c <= '9');
                    g_state.nextchar--;
                    if (negE)
                    {
                        e = e - s;
                    }
                    else
                    {
                        e = e + s;
                    }
                }
                else
                {
                    NUM_ERROR("Expected at least one digit for exponent value");
                    return nbase_token_LEXER_ERROR;
                }
            }
            if (e < 0)
            {
                if (e >= -MAX_EXPONENT)
                {
                    x = x / ten(-e);
                }
                else
                {
                    /* Too small, return 0.0. */
                    x = 0.0f;
                }
            }
            else if (e > 0)
            {
                if (e <= MAX_EXPONENT)
                {
                    x = ten(e) * x;
                }
                else
                {
                    /* Exponent too large. */
                    x = 0.0f;
                    NUM_ERROR("Number too large");
                    return nbase_token_LEXER_ERROR;
                }
            }
            g_state.fltval = x;
            return nbase_token_FLOAT_LITERAL;
        }
    }
    else
    {
        /* Base 10 integer. */
        do
        {
            if (d[i] < 10)
            {
                if (k <= (max - d[i]) / 10)
                {
                    k = k * 10 + d[i];
                }
                else
                {
                    NUM_ERROR("Number too large");
                    k = 0;
                    return nbase_token_LEXER_ERROR;
                }
            }
            else
            {
                NUM_ERROR("Bad integer");
                return nbase_token_LEXER_ERROR;
            }
            i++;
        } while (i != n);
        g_state.int32val = k;
        return nbase_token_INTEGER_LITERAL;
    }
    
    return nbase_token_LEXER_ERROR;
}

/* ******************************************************************************** */

NBASE_BOOL nbase_parse_identifier(NBASE_BOOL pVariableName)
{
    uint32_t i = 0;
    g_state.token[i++] = *g_state.nextchar++;

    /* TODO(Pablo): check for identifier name too long. */

    while (isalnum(*g_state.nextchar) || *g_state.nextchar == '_' || *g_state.nextchar == '#')
    {
        if(pVariableName && i >= 4)
            break;
        
        g_state.token[i++] = *g_state.nextchar++;
    }

    /* Check for type sentinel. */
    if(pVariableName && (*g_state.nextchar == '$' || *g_state.nextchar == '%'))
    {
        g_state.token[i++] = *g_state.nextchar++;
    }

    g_state.token[i] = '\0';

    return true;
}

/* ******************************************************************************** */

int32_t nbase_parse_single_char_token(char pTest, bool pUpdateParent)
{
    int32_t tok = 0;
    const char *tmp;
    
    if (pTest)
    {
        tmp = g_state.nextchar;
        g_state.nextchar = &pTest;
    }

    if (*g_state.nextchar == '\n')
    {
        tok = nbase_token_EOL;
        goto end;
    }
    else if (*g_state.nextchar == '(')
    {
        tok = nbase_token_LEFTPAREN;
        goto end;
    }
    else if (*g_state.nextchar == ')')
    {
        if(pUpdateParent)
        {
            g_state.paren_level--;
            NBASE_ASSERT_OR_ERROR(g_state.paren_level >= 0,
                nbase_error_type_PARENTHESES_MISMATCH, THIS_FILE, __FUNCTION__, __LINE__, "");
        }
        tok = nbase_token_RIGHTPAREN;
        goto end;
    }
    else if (*g_state.nextchar == ';')
    {
        tok = nbase_token_SEMICOLON;
        goto end;
    }
    else if (*g_state.nextchar == ':')
    {
        tok = nbase_token_COLON;
        goto end;
    }
    else if (*g_state.nextchar == ',')
    {
        tok = nbase_token_COMMA;
        goto end;
    }
    else if (*g_state.nextchar == '+')
    {
        tok = nbase_token_PLUS;
        goto end;
    }
    else if (*g_state.nextchar == '-')
    {
        tok = nbase_token_MINUS;
        goto end;
    }
    else if (*g_state.nextchar == '%')
    {
        tok = nbase_token_NEGATED;
        goto end;
    }
    else if (*g_state.nextchar == '*')
    {
        tok = nbase_token_MUL;
        goto end;
    }
    else if (*g_state.nextchar == '/')
    {
        tok = nbase_token_DIV;
        goto end;
    }
    else if (*g_state.nextchar == '=')
    {
        tok = nbase_token_EQ;
        goto end;
    }
    else if (*g_state.nextchar == '^')
    {
        tok = nbase_token_POW;
        goto end;
    }
    else if (*g_state.nextchar == '<')
    {
        if(*(g_state.nextchar + 1) == '=')
            return nbase_search_keyword_token("<=");
        if(*(g_state.nextchar + 1) == '>')
            return nbase_search_keyword_token("<>");
        tok = nbase_token_LESS;
        goto end;
    }
    else if (*g_state.nextchar == '>')
    {
        if(*(g_state.nextchar + 1) == '=')
            return nbase_search_keyword_token(">=");
        tok = nbase_token_GREATER;
        goto end;
    }

end:

    if (pTest)
    {
        g_state.nextchar = tmp;
    }

    return tok;
}

/* ******************************************************************************** */

int32_t nbase_get_next_token(NBASE_BOOL pInterpret, NBASE_BOOL pVariableName)
{
    const char *start;
    nbase_token tok = nbase_token_NONE;
    
    g_state.token[0] = '\0';
    
    nbase_skip_spaces();

    /* If we are interpreting a command, don't accept anything that's not an
        identifier. */
    if(pInterpret && !isalpha(*g_state.nextchar) && *g_state.nextchar != '_' &&
        *g_state.nextchar != '.' &&
        *g_state.nextchar != '\n' && *g_state.nextchar != '\0')
    {
        nbase_rtrim((char*)g_state.nextchar);
        nbase_error(nbase_error_type_UNEXPECTED_TOKEN,
             THIS_FILE, __FUNCTION__, __LINE__, ": %s", g_state.nextchar);
    }
    
    if (*g_state.nextchar == '.' &&
        g_state.state_flags & nbase_state_flag_TOKENIZING)
    {
        /* TODO(Pablo): do nothing for now. */
        return nbase_token_EOL;
    }
    
    if (*g_state.nextchar == '\0')
        return nbase_token_EOL;

    start = g_state.nextchar;
    
    /* Number. */
    if (isdigit(*g_state.nextchar))
    {
        return nbase_parse_number();
    }
    /* Single char token. */
    else if ((tok = nbase_parse_single_char_token(0, false)))
    {
        /* Special case for compound operators based on single chars. */
        if(tok == nbase_token_LESSEQ ||
            tok == nbase_token_GREATEREQ ||
            tok == nbase_token_NEQUALS)
        {
            g_state.nextchar += 2;
            return tok;
        }
        g_state.nextchar++;
        return nbase_parse_single_char_token(*(g_state.nextchar - 1), true);
    }
    /* String literal. */
    else if (*g_state.nextchar == '"')
    {
        uint32_t l;
        
        do
        {
            g_state.nextchar++;
        } while (*g_state.nextchar && *g_state.nextchar != '"');
        NBASE_ASSERT_OR_ERROR(*g_state.nextchar == '"',
            nbase_error_type_UNTERMINATED_STRING, THIS_FILE, __FUNCTION__, __LINE__, "");
        g_state.nextchar++;

        l = (g_state.nextchar - start) > (NBASE_MAX_TOKEN_LEN - 1) ? NBASE_MAX_TOKEN_LEN - 1 : g_state.nextchar - start;
        snprintf(g_state.token, l + 1, "%s", start);

        return nbase_token_STRING;
    }
    /* Keyword or identifier. */
    else if (isalpha(*g_state.nextchar) || *g_state.nextchar == '_' || *g_state.nextchar == '.')
    {
        nbase_keyword* kw;
        
        if(*g_state.nextchar == '.')
            pVariableName = false;
        
        nbase_parse_identifier(pVariableName);
        
        /* Search keyword. */
        kw = nbase_search_keyword(g_state.token);
        if(kw)
        {
            if(kw->func)
            {
                /* If keyword found, run its handler... */
                kw->func();
                return g_state.next_tok;
            }
            else
            {
                /* ...if kw not found, if we are interpreting then trigger error, if not, return its token. */
                if(pInterpret)
                    nbase_error(nbase_error_type_UNKNOWN_KEYWORD, THIS_FILE, __FUNCTION__, __LINE__, ": %s", start);
                return kw->token;
            }
        }
        else
        {
            /* If kw not found, if we are interpreting then trigger error. */
            if(pInterpret)
                nbase_error(nbase_error_type_EXPECTED_KEYWORD, THIS_FILE, __FUNCTION__, __LINE__, " at: ... %s", start);
        }

        /* Not a keyword... */
        return nbase_token_IDENTIFIER;
    }

    nbase_error(nbase_error_type_UNRECOGNIZED_LEXER_TOKEN, THIS_FILE, __FUNCTION__, __LINE__, " at: ... %s", start);
    return nbase_token_LEXER_ERROR;
}

/* ******************************************************************************** */

uint32_t nbase_get_size_of_type(nbase_datatype pType)
{
    switch(pType)
    {
    case nbase_datatype_FLOAT:
        return sizeof(NBASE_FLOAT);
        break;
        
    case nbase_datatype_INTEGER:
        return sizeof(NBASE_INTEGER);
        break;

    case nbase_datatype_STRING:
        return sizeof(void*);
        break;
        
    default:
        NBASE_ASSERT_OR_INTERNAL_ERROR(0,
            "nbase_get_size_of_type(): UNKNOWN TYPE", THIS_FILE, __FUNCTION__, __LINE__);
    }

    return 0;
}

/* ******************************************************************************** */

void nbase_reset_state()
{
    int32_t num_objs;
    
    /* Clear input buffer. */
    g_state.nextchar = g_state.input_buffer;
    memset(g_state.input_buffer, 0, NBASE_MAX_LINE_LEN_PLUS_1);
    
    /* Clear line count, some parser state. */
    g_state.in_line = 0;
    g_state.n_lines = 0;
    g_state.int32val = 0;
    g_state.fltval = 0.0f;

    /* Clear paren level. */
    g_state.paren_level = 0;
    
    /* Turn off tokenizing flag. */
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
