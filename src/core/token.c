/*
 * token.c
 * Tokenization.
 *
 * 03 nov 2022 -- 17:41 -03
 * Notes:
 */

#ifdef TOKEN_DEFINITIONS
#undef TOKEN_DEFINITIONS

/* -------------------------------------------------------------------------------- */
/* Prototypes. */

const char*                             nbase_token_code_to_name(nbase_token pCode);
void                                    nbase_tokenize_keyword(nbase_token pCode);
void                                    nbase_tokenize_factor(nbase_ast_node* pNode);
void                                    nbase_tokenize_var(nbase_datatype pType, const char* pName);
#endif /* TOKEN_DEFINITIONS */

/* -------------------------------------------------------------------------------- */
/* Implementation. */

#ifdef TOKEN_IMPLEMENTATION
#undef TOKEN_IMPLEMENTATION

const char* TOKEN_FILE = "token.c";

/* ******************************************************************************** */

const char* nbase_token_code_to_name(nbase_token pCode)
{
    switch(pCode)
    {
    case nbase_token_PRINT:            return "PRINT";
    case nbase_token_PLUS:             return "+";
    case nbase_token_MINUS:            return "-";
    case nbase_token_MUL:              return "*";
    case nbase_token_DIV:              return "/";
    case nbase_token_LEFTPAREN:        return "(";
    case nbase_token_RIGHTPAREN:       return ")";
    case nbase_token_COLON:            return ":";
    case nbase_token_SEMICOLON:        return ";";
    case nbase_token_EQ:               return "=";
    case nbase_token_COMMA:            return ",";
    case nbase_token_POW:              return "^";

    case nbase_token_REM:              return "REM";
    case nbase_token_DIM:              return "DIM";
    case nbase_token_LET:              return "LET";
    case nbase_token_NEGATED:          return "%";

    case nbase_token_MOD:              return "MOD";

    case nbase_token_LSHIFT:           return "LSHIFT";
    case nbase_token_RSHIFT:           return "RSHIFT";
    case nbase_token_LESS:             return "<";
    case nbase_token_LESSEQ:           return "<=";
    case nbase_token_GREATER:          return ">";
    case nbase_token_GREATEREQ:        return ">=";
    case nbase_token_NEQUALS:          return "<>";
    
    case nbase_token_AND:              return "AND";
    case nbase_token_OR:               return "OR";
    case nbase_token_XOR:              return "XOR";
    case nbase_token_NOT:              return "NOT";
    case nbase_token_GC:               return "GC";
    case nbase_token_END:              return "END";
    /*case nbase_token_CLEAR:            return "CLEAR";*/
    case nbase_token_LIST:             return "LIST";
    /*case nbase_token_GOTO:             return "GOTO";*/
    case nbase_token_LOADLINE:         return "LOADLINE";

    case nbase_token_NL:               return "NL";
    case nbase_token_EOL:              return "EOL";
    
#ifdef NBASE_INCLUDE_FEATURE_DEBUGTOOLS
    case nbase_token_LVAR:              return "LVAR";
    case nbase_token_STAT:              return "STAT";
    case nbase_token_DUMP:              return "DUMP";
#endif /* NBASE_INCLUDE_FEATURE_DEBUGTOOLS */

    default:                            return "<UNKNOWN TOKEN>";
    }
}

/* ******************************************************************************** */

void nbase_tokenize_keyword(nbase_token pCode)
{
    if(pCode >= 33 && pCode < nbase_token_LAST)
    {
        uint16_t* p = (uint16_t*)g_state.code_limit;

#ifdef NBASE_DEBUG_TOKENIZER
        NBASE_PRINTF("tokenize_keyword: %p = 0%XH %s\n", (void*)p, pCode, nbase_token_code_to_name(pCode));
#endif /* NBASE_DEBUG_TOKENIZER */

        *p++ = pCode;
        g_state.code_limit = (uint8_t*)p;
        return;
    }

    NBASE_ASSERT_OR_INTERNAL_ERROR(0,
        "UNKNOWN TOKEN CODE", TOKEN_FILE, __FUNCTION__, __LINE__);
}

/* ******************************************************************************** */

void nbase_tokenize_factor(nbase_ast_node* pNode)
{
    uint8_t* p8 = (uint8_t*)g_state.code_limit;
    NBASE_INTEGER* pint;
    NBASE_FLOAT* pflt;
    
    switch(pNode->data_type)
    {
    case nbase_datatype_FLOAT:
#ifdef NBASE_DEBUG_TOKENIZER
        NBASE_PRINTF("tokenize_factor:  %p = '&'\n", (void*)p8);
#endif /* NBASE_DEBUG_TOKENIZER */
        *p8++ = '&';
        pflt = (NBASE_FLOAT*)p8;
#ifdef NBASE_DEBUG_TOKENIZER
        NBASE_PRINTF("tokenize_factor:  %p = %.50f\n", (void*)pflt, pNode->u.flt_val);
#endif /* NBASE_DEBUG_TOKENIZER */
        *pflt++ = pNode->u.flt_val;
        g_state.code_limit = (uint8_t*)pflt;
        break;
        
    case nbase_datatype_INTEGER:
#ifdef NBASE_DEBUG_TOKENIZER
        NBASE_PRINTF("tokenize_factor:  %p = '!'\n", (void*)p8);
#endif /* NBASE_DEBUG_TOKENIZER */
        *p8++ = '!';
        pint = (NBASE_INTEGER*)p8;
#ifdef NBASE_DEBUG_TOKENIZER
        NBASE_PRINTF("tokenize_factor:  %p = %d\n", (void*)pint, pNode->u.int_val);
#endif /* NBASE_DEBUG_TOKENIZER */
        *pint++ = pNode->u.int_val;
        g_state.code_limit = (uint8_t*)pint;
        break;

    case nbase_datatype_STRING:
#ifdef NBASE_DEBUG_TOKENIZER
        NBASE_PRINTF("tokenize_factor:  %p = '$'\n", (void*)p8);
#endif /* NBASE_DEBUG_TOKENIZER */
        *p8++ = '$';
#ifdef NBASE_DEBUG_TOKENIZER
        NBASE_PRINTF("tokenize_factor:  %p = \"%s\" (%lu)\n", (void*)p8, pNode->u.str_val, strlen(pNode->u.str_val) + 1);
#endif /* NBASE_DEBUG_TOKENIZER */
        strcpy((char*)p8, pNode->u.str_val);
        p8 += strlen(pNode->u.str_val);
        *p8++ = '\0';
        g_state.code_limit = p8;
        break;

    default:
        NBASE_ASSERT_OR_INTERNAL_ERROR(0,
            "UNKNOWN DATA TYPE FOR AST NODE", TOKEN_FILE, __FUNCTION__, __LINE__);
    }
}

/* ******************************************************************************** */

void nbase_tokenize_var(nbase_datatype pType, const char* pName)
{
    uint8_t* p8 = (uint8_t*)g_state.code_limit;

    switch(pType)
    {
    case nbase_datatype_FLOAT:
#ifdef NBASE_DEBUG_TOKENIZER
        NBASE_PRINTF("tokenize_var:  %p = '@'\n", (void*)p8);
        NBASE_PRINTF("tokenize_var:  %p = '&'\n", (void*)(p8 + 1));
#endif /* NBASE_DEBUG_TOKENIZER */
        *p8++ = '@';
        *p8++ = '&';
        break;
    case nbase_datatype_INTEGER:
#ifdef NBASE_DEBUG_TOKENIZER
        NBASE_PRINTF("tokenize_var:  %p = '@'\n", (void*)p8);
        NBASE_PRINTF("tokenize_var:  %p = '!'\n", (void*)(p8 + 1));
#endif /* NBASE_DEBUG_TOKENIZER */
        *p8++ = '@';
        *p8++ = '!';
        break;
    case nbase_datatype_STRING:
#ifdef NBASE_DEBUG_TOKENIZER
        NBASE_PRINTF("tokenize_var:  %p = '@'\n", (void*)p8);
        NBASE_PRINTF("tokenize_var:  %p = '$'\n", (void*)(p8 + 1));
#endif /* NBASE_DEBUG_TOKENIZER */
        *p8++ = '@';
        *p8++ = '$';
        break;
    
    default:
        NBASE_ASSERT_OR_INTERNAL_ERROR(0,
            "nbase_tokenize_var(): UNKNOWN DATA TYPE", TOKEN_FILE, __FUNCTION__, __LINE__);
        break;
    }

#ifdef NBASE_DEBUG_TOKENIZER
        NBASE_PRINTF("tokenize_var:  %p = \"%s\" (%lu)\n", (void*)p8, pName, strlen(pName) + 1);
#endif /* NBASE_DEBUG_TOKENIZER */
        strcpy((char*)p8, pName);
        p8 += strlen(pName);
        *p8++ = '\0';
        g_state.code_limit = p8;
}

#endif /* TOKEN_IMPLEMENTATION */
