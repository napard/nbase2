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

const char* nbase_token_code_to_name(nbase_token pCode);
void nbase_tokenize_keyword             (nbase_token pCode);

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

    default:                           return "<UNKNOWN TOKEN>";
    }
}

/* ******************************************************************************** */

void nbase_tokenize_keyword(nbase_token pCode)
{
    if(pCode >= nbase_token_PLUS && pCode < nbase_token_LAST)
    {
        uint16_t* p = (uint16_t*)g_state.code_limit;

#ifdef NBASE_DEBUG
        NBASE_PRINTF("tokenize_keyword: %p = 0%XH %s\n", (void*)p, pCode, nbase_token_code_to_name(pCode));
#endif /* NBASE_DEBUG */

        *p++ = pCode;
        g_state.code_limit = (uint8_t*)p;
        return;
    }

    NBASE_ASSERT_OR_INTERNAL_ERROR(0,
        "UNKNOWN TOKEN CODE", TOKEN_FILE, __FUNCTION__, __LINE__);
}

#endif /* TOKEN_IMPLEMENTATION */
