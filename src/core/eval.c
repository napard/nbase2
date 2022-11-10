/*
 * eval.c
 * 
 *
 * 04 nov 2022 -- 18:16 -03
 * Notes:
 */

#ifdef EVAL_DEFINITIONS
#undef EVAL_DEFINITIONS

/* -------------------------------------------------------------------------------- */
/* Type definitions. */

struct _nbase_eval_node;

typedef struct _nbase_eval_op
{
    struct _nbase_eval_node* lhs;
    struct _nbase_eval_node* rhs;
    uint16_t oper;
} nbase_eval_op;

typedef struct _nbase_eval_node
{
    nbase_datatype data_type;
    union
    {
        char* str_val;
        NBASE_INTEGER int_val;
        NBASE_FLOAT flt_val;
        nbase_eval_op op;
        void* extra; /* Variable reference. */
    } v;
} nbase_eval_node;

#define PTR16(x) ((uint16_t*)x)

/* -------------------------------------------------------------------------------- */
/* Prototypes. */

void                                    nbase_eval_term(nbase_eval_node* pNodeLeft, nbase_eval_node* pNodeRight,
                                            uint16_t pOper, uint8_t* pPtr, nbase_eval_node* pNodeOut);
void                                    nbase_eval_sum(nbase_eval_node* pNodeLeft, nbase_eval_node* pNodeRight,
                                            uint16_t pOper, uint8_t* pPtr, nbase_eval_node* pNodeOut);
void                                    nbase_eval_logic_op(nbase_eval_node* pNodeLeft, nbase_eval_node* pNodeRight,
                                            uint16_t pOper, uint8_t* pPtr, nbase_eval_node* pNodeOut);
void                                    nbase_eval_expression(nbase_eval_node* pNodeLeft, nbase_eval_node* pNodeRight,
                                            uint16_t pOper, uint8_t* pPtr, nbase_eval_node* pNodeOut);

#endif /* EVAL_DEFINITIONS */
 
/* -------------------------------------------------------------------------------- */
/* Implementation. */

#ifdef EVAL_IMPLEMENTATION
#undef EVAL_IMPLEMENTATION

const char* EVAL_FILE = "eval.c";

/* ******************************************************************************** */

void nbase_eval_term(nbase_eval_node* pNodeLeft, nbase_eval_node* pNodeRight,
    uint16_t pOper, uint8_t* pPtr, nbase_eval_node* pNodeOut)
{
    nbase_eval_node node, node2;
    nbase_token unary = nbase_token_NONE;
    
    /* Take into account unary operators (%, NOT, etc.), necessarily skip spaces. */
    if(*PTR16(pPtr) == nbase_token_NEGATED ||       /* %   */
        *PTR16(pPtr) == nbase_token_NEGATED)        /* NOT */
    {
        unary = *pPtr;
    }
    else {
        //nbase_eval_factor(pNodeLeft, &node);
    }

    /* Loop while a valid operation inside a term is found... */
    while (*PTR16(pPtr) == nbase_token_MUL ||       /* MUL */
        *PTR16(pPtr) == nbase_token_DIV ||          /* DIV */
        *PTR16(pPtr) == nbase_token_MOD ||          /* MOD */
        *PTR16(pPtr) == nbase_token_NEGATED ||      /* %   */
        *PTR16(pPtr) == nbase_token_POW ||          /* ^   */
        *PTR16(pPtr) == nbase_token_NOT             /* NOT */
    )
    {
        pPtr += 2;
        //nbase_eval_factor(pNodeLeft, &node2);
    }
}

/* ******************************************************************************** */

void nbase_eval_sum(nbase_eval_node* pNodeLeft, nbase_eval_node* pNodeRight,
    uint16_t pOper, uint8_t* pPtr, nbase_eval_node* pNodeOut)
{
    nbase_eval_node node, node2;
    nbase_eval_term(pNodeLeft, pNodeRight, pOper, pPtr, &node);
    
    /* Loop while a valid operation inside a sum is found... */
    while (*PTR16(pPtr) == nbase_token_PLUS ||      /* +      */
        *PTR16(pPtr) == nbase_token_MINUS ||        /* -      */
        *PTR16(pPtr) == nbase_token_LSHIFT ||       /* LSHIFT */
        *PTR16(pPtr) == nbase_token_RSHIFT          /* RSHIFT */
    )
    {
        pPtr += 2;
        nbase_eval_term(&node, NULL, *pPtr, pPtr, &node2);
    }
}

/* ******************************************************************************** */

void nbase_eval_logic_op(nbase_eval_node* pNodeLeft, nbase_eval_node* pNodeRight,
    uint16_t pOper, uint8_t* pPtr, nbase_eval_node* pNodeOut)
{
    nbase_eval_node node, node2;
    nbase_eval_sum(pNodeLeft, pNodeRight, pOper, pPtr, &node);
    
    /* Loop while a valid operation inside a logic op is found... */
    while (*PTR16(pPtr) == nbase_token_AND ||       /* AND */
        *PTR16(pPtr) == nbase_token_OR ||           /* OR  */
        *PTR16(pPtr) == nbase_token_XOR             /* XOR */
    )
    {
        pPtr += 2;
        nbase_eval_sum(&node, NULL, *pPtr, pPtr, &node2);
    }
}
                                            
/* ******************************************************************************** */

void nbase_eval_expression(nbase_eval_node* pNodeLeft, nbase_eval_node* pNodeRight,
    uint16_t pOper, uint8_t* pPtr, nbase_eval_node* pNodeOut)
{
    nbase_eval_node node, node2;
    nbase_eval_logic_op(pNodeLeft, pNodeRight, pOper, pPtr, &node);
    
    /* Loop while a valid operation inside an expression is found... */
    while (*PTR16(pPtr) == nbase_token_LESS ||      /* <  */
        *PTR16(pPtr) == nbase_token_LESSEQ ||       /* <= */
        *PTR16(pPtr) == nbase_token_GREATER ||      /* >  */
        *PTR16(pPtr) == nbase_token_GREATEREQ ||    /* >= */
        *PTR16(pPtr) == nbase_token_EQ ||           /* =  */
        *PTR16(pPtr) == nbase_token_NEQUALS         /* <> */
    )
    {
        pPtr += 2;
        nbase_eval_logic_op(&node, NULL, *pPtr, pPtr, &node2);
    }
}

#endif /* EVAL_IMPLEMENTATION */
