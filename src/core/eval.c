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

void                                    nbase_build_node(uint8_t** pPtr, nbase_eval_node* pNodeOut);
void                                    nbase_eval_factor(nbase_eval_node* pNode, nbase_eval_node* pNodeOut);
void                                    nbase_eval_term(nbase_eval_node* pNodeLeft, /*nbase_eval_node* pNodeRight,
                                            uint16_t pOper,*/ uint8_t** pPtr, nbase_eval_node* pNodeOut);
void                                    nbase_eval_sum(nbase_eval_node* pNodeLeft, /*nbase_eval_node* pNodeRight,
                                            uint16_t pOper,*/ uint8_t** pPtr, nbase_eval_node* pNodeOut);
void                                    nbase_eval_logic_op(nbase_eval_node* pNodeLeft, /*nbase_eval_node* pNodeRight,
                                            uint16_t pOper,*/ uint8_t** pPtr, nbase_eval_node* pNodeOut);
void                                    nbase_eval_expression(nbase_eval_node* pNodeLeft, /*nbase_eval_node* pNodeRight,
                                            uint16_t pOper,*/ uint8_t** pPtr, nbase_eval_node* pNodeOut);

#endif /* EVAL_DEFINITIONS */
 
/* -------------------------------------------------------------------------------- */
/* Implementation. */

#ifdef EVAL_IMPLEMENTATION
#undef EVAL_IMPLEMENTATION

const char* EVAL_FILE = "eval.c";

/* ******************************************************************************** */

void nbase_build_node(uint8_t** pPtr, nbase_eval_node* pNodeOut)
{
    uint8_t* cp = *pPtr;
    switch(*cp++)
    {
    case '&':
        pNodeOut->data_type = nbase_datatype_FLOAT;
        pNodeOut->v.flt_val = *((NBASE_FLOAT*)cp);
        (*pPtr) += sizeof(NBASE_FLOAT) + 1/* - 2*/;
        break;
    case '!':
        pNodeOut->data_type = nbase_datatype_INTEGER;
        pNodeOut->v.int_val = *((NBASE_INTEGER*)cp);
        (*pPtr) += sizeof(NBASE_INTEGER) + 1/* - 2*/;
        break;
    case '$':
        pNodeOut->data_type = nbase_datatype_STRING;
        pNodeOut->v.str_val = (char*)cp;
        /* TODO: INCREMENT POINTER */
        break;
    
    default:
        NBASE_ASSERT_OR_INTERNAL_ERROR(0,
            "nbase_build_node(): UNKNOWN DATA TYPE", EVAL_FILE, __FUNCTION__, __LINE__);
    }
}

/* ******************************************************************************** */

void nbase_eval_factor(nbase_eval_node* pNode, nbase_eval_node* pNodeOut)
{
    switch(pNode->data_type)
    {
    case nbase_datatype_INTEGER:
    case nbase_datatype_FLOAT:
    case nbase_datatype_STRING:
        *pNodeOut = *pNode;
        break;
    
    default:
        NBASE_ASSERT_OR_INTERNAL_ERROR(0,
            "nbase_eval_factor(): UNKNOWN DATA TYPE", EVAL_FILE, __FUNCTION__, __LINE__);
    }
}

/* ******************************************************************************** */

void nbase_eval_term(nbase_eval_node* pNodeLeft, /*nbase_eval_node* pNodeRight,
    uint16_t pOper,*/ uint8_t** pPtr, nbase_eval_node* pNodeOut)
{
    nbase_eval_node node, node2, node_tmp;
    nbase_token unary = nbase_token_NONE;
    
    /* Take into account unary operators (%, NOT, etc.) */
    if(*PTR16(*pPtr) == nbase_token_NEGATED ||       /* %   */
        *PTR16(*pPtr) == nbase_token_NEGATED)        /* NOT */
    {
        unary = *PTR16(*pPtr);
    }
    else {
        nbase_eval_factor(pNodeLeft, &node);
    }

    /* Loop while a valid operation inside a term is found... */
    while (*PTR16(*pPtr) == nbase_token_MUL ||       /* MUL */
        *PTR16(*pPtr) == nbase_token_DIV ||          /* DIV */
        *PTR16(*pPtr) == nbase_token_MOD ||          /* MOD */
        *PTR16(*pPtr) == nbase_token_NEGATED ||      /* %   */
        *PTR16(*pPtr) == nbase_token_POW ||          /* ^   */
        *PTR16(*pPtr) == nbase_token_NOT             /* NOT */
    )
    {
        int16_t oper = *PTR16(*pPtr);
        *pPtr += 2;
        
        nbase_build_node(pPtr, &node_tmp);
        nbase_eval_factor(pNodeLeft, &node2);
    }

    *pNodeOut = node;
}

/* ******************************************************************************** */

void nbase_eval_sum(nbase_eval_node* pNodeLeft, /*nbase_eval_node* pNodeRight,
    uint16_t pOper,*/ uint8_t** pPtr, nbase_eval_node* pNodeOut)
{
    nbase_eval_node node, node2, node_tmp;
    nbase_eval_term(pNodeLeft, /*pNodeRight, pOper,*/ pPtr, &node);
    
    /* Loop while a valid operation inside a sum is found... */
    while (*PTR16(*pPtr) == nbase_token_PLUS ||      /* +      */
        *PTR16(*pPtr) == nbase_token_MINUS ||        /* -      */
        *PTR16(*pPtr) == nbase_token_LSHIFT ||       /* LSHIFT */
        *PTR16(*pPtr) == nbase_token_RSHIFT          /* RSHIFT */
    )
    {
        int16_t oper = *PTR16(*pPtr);
        *pPtr += 2;
        
        nbase_build_node(pPtr, &node_tmp);
        nbase_eval_term(&node_tmp, /*NULL, *pPtr,*/ pPtr, &node2);

        switch(node.data_type)
        {
        case nbase_datatype_FLOAT:
            /* FLOAT ************************************************************ */
            switch(oper)
            {
            case nbase_token_PLUS:
                node.v.flt_val += node2.v.flt_val;
                break;

            case nbase_token_MINUS:
                node.v.flt_val -= node2.v.flt_val;
                break;

            default:
                break;
            }
            /* FLOAT ******************************************************** END */
            break;

        case nbase_datatype_INTEGER:
            /* INTEGER ********************************************************** */
            switch(oper)
            {
            case nbase_token_PLUS:
                node.v.int_val += node2.v.int_val;
                break;

            case nbase_token_MINUS:
                node.v.int_val -= node2.v.int_val;
                break;

            case nbase_token_LSHIFT:
                node.v.int_val <<= node2.v.int_val;
                break;

            case nbase_token_RSHIFT:
                node.v.int_val >>= node2.v.int_val;
                break;

            default:
                break;
            }
            /* INTEGER ****************************************************** END */
            break;

        default:
            break;
        }
    }

    *pNodeOut = node;
}

/* ******************************************************************************** */

void nbase_eval_logic_op(nbase_eval_node* pNodeLeft, /*nbase_eval_node* pNodeRight,
    uint16_t pOper,*/ uint8_t** pPtr, nbase_eval_node* pNodeOut)
{
    nbase_eval_node node, node2, node_tmp;
    nbase_eval_sum(pNodeLeft, /*pNodeRight, pOper,*/ pPtr, &node);
    
    /* Loop while a valid operation inside a logic op is found... */
    while (*PTR16(*pPtr) == nbase_token_AND ||       /* AND */
        *PTR16(*pPtr) == nbase_token_OR ||           /* OR  */
        *PTR16(*pPtr) == nbase_token_XOR             /* XOR */
    )
    {
        int16_t oper = *PTR16(*pPtr);
        *pPtr += 2;
        
        nbase_build_node(pPtr, &node_tmp);
        nbase_eval_sum(&node_tmp, /*NULL, *pPtr,*/ pPtr, &node2);

        switch(node.data_type)
        {
        case nbase_datatype_INTEGER:
            /* INTEGER ********************************************************** */
            switch(oper)
            {
            case nbase_token_AND:
                node.v.int_val &= node2.v.int_val;
                break;

            case nbase_token_OR:
                node.v.int_val |= node2.v.int_val;
                break;

            case nbase_token_XOR:
                node.v.int_val ^= node2.v.int_val;
                break;

            default:
                break;
            }
            /* INTEGER ****************************************************** END */
            break;

        default:
            break;
        }
    }

    *pNodeOut = node;
}
                                            
/* ******************************************************************************** */

void nbase_eval_expression(nbase_eval_node* pNodeLeft, /*nbase_eval_node* pNodeRight,
    uint16_t pOper,*/ uint8_t** pPtr, nbase_eval_node* pNodeOut)
{
    nbase_eval_node node, node2;
    nbase_eval_logic_op(pNodeLeft, /*pNodeRight, pOper,*/ pPtr, &node);
    
    /* Loop while a valid operation inside an expression is found... */
    while (*PTR16(*pPtr) == nbase_token_LESS ||      /* <  */
        *PTR16(*pPtr) == nbase_token_LESSEQ ||       /* <= */
        *PTR16(*pPtr) == nbase_token_GREATER ||      /* >  */
        *PTR16(*pPtr) == nbase_token_GREATEREQ ||    /* >= */
        *PTR16(*pPtr) == nbase_token_EQ ||           /* =  */
        *PTR16(*pPtr) == nbase_token_NEQUALS         /* <> */
    )
    {
        int16_t oper = *PTR16(*pPtr);
        *pPtr += 2;
        nbase_eval_logic_op(&node, /*NULL, *pPtr,*/ pPtr, &node2);
    }

    *pNodeOut = node;
}

#endif /* EVAL_IMPLEMENTATION */
