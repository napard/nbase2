/*
 * oper.c
 * Operators.
 *
 * 21 sep 2022 -- 11:28 -03
 * Notes:
 */

/* -------------------------------------------------------------------------------- */
/* Implementation. */

#ifdef OPER_IMPLEMENTATION
#undef OPER_IMPLEMENTATION

const char* OPER_FILE = "oper.c";

struct
{
    uint16_t oper_id;
    const char* name;
} operators[] =
{
    { nbase_token_PLUS,           "+" },
    { nbase_token_MINUS,          "-" },
    { nbase_token_MUL,            "*" },
    { nbase_token_DIV,            "/" },
    { nbase_token_MOD,            "MOD" },
    { nbase_token_POW,            "^" },
    { nbase_token_NEGATED,        "%" },

    { nbase_token_LSHIFT,         "LSHIFT" },
    { nbase_token_RSHIFT,         "RSHIFT" },
    { nbase_token_LESS,           "<" },
    { nbase_token_LESSEQ,         "<=" },
    { nbase_token_GREATER,        ">" },
    { nbase_token_GREATEREQ,      ">=" },
    { nbase_token_EQ,             "=" },
    { nbase_token_NEQUALS,        "<>" },
    
    { nbase_token_AND,            "AND" },
    { nbase_token_OR,             "OR" },
    { nbase_token_XOR,            "XOR" },
    { nbase_token_NOT,            "NOT" }
};

const char* nbase_get_oper_name(uint16_t pOper)
{
    uint32_t i;
    for(i = 0; i < sizeof(operators); i++)
    {
        if(pOper == operators[i].oper_id)
            return operators[i].name;
    }
    
    NBASE_ASSERT_OR_INTERNAL_ERROR(0, "UNKNOWN OPERATOR: %d",
        OPER_FILE, __FUNCTION__, __LINE__, pOper);

    return NULL;
}

#endif /* OPER_IMPLEMENTATION */


