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

typedef struct _nbase_eval_node
{
    nbase_datatype data_type;
    union
    {
        char* str_val;
        NBASE_INTEGER int_val;
        NBASE_FLOAT flt_val;
        void* extra; /* Variable reference. */
    } v;
} nbase_eval_node;

#define PTR16(x) ((uint16_t*)x)

/* -------------------------------------------------------------------------------- */
/* Prototypes. */

nbase_token                             nbase_build_node(uint8_t** pPtr, nbase_eval_node* pNodeOut);
void                                    nbase_eval_factor(nbase_eval_node* pNode, nbase_eval_node* pNodeOut);
void                                    nbase_eval_term(nbase_token pUnary, nbase_eval_node* pNodeLeft,
                                            uint8_t** pPtr, nbase_eval_node* pNodeOut);
void                                    nbase_eval_sum(nbase_token pUnary, nbase_eval_node* pNodeLeft,
                                            uint8_t** pPtr, nbase_eval_node* pNodeOut);
void                                    nbase_eval_logic_op(nbase_token pUnary, nbase_eval_node* pNodeLeft,
                                            uint8_t** pPtr, nbase_eval_node* pNodeOut);
void                                    nbase_eval_expression(nbase_token pUnary, nbase_eval_node* pNodeLeft,
                                            uint8_t** pPtr, nbase_eval_node* pNodeOut);

#endif /* EVAL_DEFINITIONS */
 
/* -------------------------------------------------------------------------------- */
/* Implementation. */

#ifdef EVAL_IMPLEMENTATION
#undef EVAL_IMPLEMENTATION

const char* EVAL_FILE = "eval.c";

/* ******************************************************************************** */

nbase_token nbase_build_node(uint8_t** pPtr, nbase_eval_node* pNodeOut)
{
    uint8_t* cp = *pPtr;
    nbase_token unary = nbase_token_NONE;

    if(*((uint16_t*)cp) == nbase_token_NEGATED || *((uint16_t*)cp) == nbase_token_NOT)
    {
        unary = *((uint16_t*)cp);
        cp += 2;
        (*pPtr) += 2;
    }
    
    switch(*cp++)
    {
    case '&':
        pNodeOut->data_type = nbase_datatype_FLOAT;
        pNodeOut->v.flt_val = *((NBASE_FLOAT*)cp);
        (*pPtr) += sizeof(NBASE_FLOAT) + 1;
        break;
    case '!':
        pNodeOut->data_type = nbase_datatype_INTEGER;
        pNodeOut->v.int_val = *((NBASE_INTEGER*)cp);
        (*pPtr) += sizeof(NBASE_INTEGER) + 1;
        break;
    case '$':
        pNodeOut->data_type = nbase_datatype_STRING;
        pNodeOut->v.str_val = (char*)cp;
        /* TODO: INCREMENT POINTER */
        break;

    case '(':
        {
            cp++;
            nbase_eval_node node;
            nbase_token unary = nbase_build_node(&cp, &node);

            /* Call expression evaluator. */
            nbase_eval_expression(unary, &node, &cp, pNodeOut);
            /* NOTE(Pablo): matching parentheses should has been already handled by the parser. */
            *pPtr = cp + 2;
        }
        break;
    
    default:
        NBASE_ASSERT_OR_INTERNAL_ERROR(0,
            "nbase_build_node(): UNKNOWN DATA TYPE", EVAL_FILE, __FUNCTION__, __LINE__);
    }

    return unary;
}

/* ******************************************************************************** */

void nbase_eval_factor(nbase_eval_node* pNode, nbase_eval_node* pNodeOut)
{
    /* TODO: take into account variables and other... (see 'nbase_parse_factor' as reference). */
    
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

void nbase_eval_term(nbase_token pUnary, nbase_eval_node* pNodeLeft,
    uint8_t** pPtr, nbase_eval_node* pNodeOut)
{
    nbase_eval_node node, node2, node_tmp;
#if 0
    nbase_token unary = nbase_token_NONE;
    
    /* Take into account unary operators (%, NOT, etc.) */
    if(*PTR16(*pPtr) == nbase_token_NEGATED ||       /* %   */
        *PTR16(*pPtr) == nbase_token_NEGATED)        /* NOT */
    {
        unary = *PTR16(*pPtr);
    }
    else {
#endif
        nbase_eval_factor(pNodeLeft, &node);
#if 0
    }
#endif

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
        
        /* TODO(Pablo): EVAL UNARY HERE??? */
        nbase_build_node(pPtr, &node_tmp);
        nbase_eval_factor(&node_tmp, &node2);

        switch(node.data_type)
        {
        case nbase_datatype_FLOAT:
            /* FLOAT ************************************************************ */
            switch(oper)
            {
            case nbase_token_MUL:
                node.v.flt_val *= node2.v.flt_val;
                break;

            case nbase_token_DIV:
                if(node2.v.flt_val == 0)
                    nbase_error(nbase_error_type_DIVISION_BY_ZERO, EVAL_FILE, __FUNCTION__, __LINE__, "");
                node.v.flt_val /= node2.v.flt_val;
                break;

            case nbase_token_POW:
            {
                errno = 0; feclearexcept(FE_ALL_EXCEPT);
                node.v.flt_val = pow(node.v.flt_val, node2.v.flt_val);
                if(errno == EDOM) perror("    errno == EDOM");
                if(fetestexcept(FE_INVALID))
                    nbase_error(nbase_error_type_DOM_ERROR_1, EVAL_FILE, __FUNCTION__, __LINE__, "");
                if(fetestexcept(FE_DIVBYZERO))
                    nbase_error(nbase_error_type_DOM_ERROR_2, EVAL_FILE, __FUNCTION__, __LINE__, "");
            }
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
            case nbase_token_MUL:
                node.v.int_val *= node2.v.int_val;
                break;

            case nbase_token_DIV:
                if(node2.v.int_val == 0)
                    nbase_error(nbase_error_type_DIVISION_BY_ZERO, EVAL_FILE, __FUNCTION__, __LINE__, "");
                node.v.int_val /= node2.v.int_val;
                break;

            case nbase_token_MOD:
                if(node2.v.int_val == 0)
                    nbase_error(nbase_error_type_DIVISION_BY_ZERO, EVAL_FILE, __FUNCTION__, __LINE__, "");
                node.v.int_val %= node2.v.int_val;
                break;

            case nbase_token_POW:
            {
                errno = 0; feclearexcept(FE_ALL_EXCEPT);
                node.v.int_val = pow(node.v.int_val, node2.v.int_val);
                if(errno == EDOM) perror("    errno == EDOM");
                if(fetestexcept(FE_INVALID))
                    nbase_error(nbase_error_type_DOM_ERROR_1, EVAL_FILE, __FUNCTION__, __LINE__, "");
                if(fetestexcept(FE_DIVBYZERO))
                    nbase_error(nbase_error_type_DOM_ERROR_2, EVAL_FILE, __FUNCTION__, __LINE__, "");
            }
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

    /* Apply unary operator. */
    if(pUnary != nbase_token_NONE)
    {
        if(pUnary == nbase_token_NEGATED)
        {
            switch(node.data_type)
            {
            case nbase_datatype_FLOAT:
                node.v.flt_val = -node.v.flt_val;
                break;

            case nbase_datatype_INTEGER:
                node.v.int_val = -node.v.int_val;
                break;
            
            default:
                break;
            }
        }
        else if(pUnary == nbase_token_NOT)
        {
            switch(node.data_type)
            {
            case nbase_datatype_FLOAT:
                node.v.flt_val = !node.v.flt_val;
                break;

            case nbase_datatype_INTEGER:
                node.v.int_val = !node.v.int_val;
                break;
            
            default:
                break;
            }
        }
    }
    
    *pNodeOut = node;
}

/* ******************************************************************************** */

void nbase_eval_sum(nbase_token pUnary, nbase_eval_node* pNodeLeft,
    uint8_t** pPtr, nbase_eval_node* pNodeOut)
{
    nbase_eval_node node, node2, node_tmp;
    nbase_eval_term(pUnary, pNodeLeft, pPtr, &node);
    
    /* Loop while a valid operation inside a sum is found... */
    while (*PTR16(*pPtr) == nbase_token_PLUS ||      /* +      */
        *PTR16(*pPtr) == nbase_token_MINUS ||        /* -      */
        *PTR16(*pPtr) == nbase_token_LSHIFT ||       /* LSHIFT */
        *PTR16(*pPtr) == nbase_token_RSHIFT          /* RSHIFT */
    )
    {
        int16_t oper = *PTR16(*pPtr);
        *pPtr += 2;
        
        nbase_token unary2 = nbase_build_node(pPtr, &node_tmp);
        nbase_eval_term(unary2, &node_tmp, pPtr, &node2);

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

void nbase_eval_logic_op(nbase_token pUnary, nbase_eval_node* pNodeLeft,
    uint8_t** pPtr, nbase_eval_node* pNodeOut)
{
    nbase_eval_node node, node2, node_tmp;
    nbase_eval_sum(pUnary, pNodeLeft, pPtr, &node);
    
    /* Loop while a valid operation inside a logic op is found... */
    while (*PTR16(*pPtr) == nbase_token_AND ||       /* AND */
        *PTR16(*pPtr) == nbase_token_OR ||           /* OR  */
        *PTR16(*pPtr) == nbase_token_XOR             /* XOR */
    )
    {
        int16_t oper = *PTR16(*pPtr);
        *pPtr += 2;
        
        nbase_token unary2 = nbase_build_node(pPtr, &node_tmp);
        nbase_eval_sum(unary2, &node_tmp, pPtr, &node2);

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

void nbase_eval_expression(nbase_token pUnary, nbase_eval_node* pNodeLeft,
    uint8_t** pPtr, nbase_eval_node* pNodeOut)
{
    nbase_eval_node node, node2, node_tmp;
    nbase_eval_logic_op(pUnary, pNodeLeft, pPtr, &node);
    
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

        nbase_token unary2 = nbase_build_node(pPtr, &node_tmp);
        nbase_eval_logic_op(unary2, &node_tmp, pPtr, &node2);
    
        switch(node.data_type)
        {
        case nbase_datatype_FLOAT:
            /* INTEGER ********************************************************** */
            switch(oper)
            {
            case nbase_token_LESS:
                node.v.flt_val = node.v.flt_val < node2.v.flt_val;
                break;

            case nbase_token_LESSEQ:
                node.v.flt_val = node.v.flt_val <= node2.v.flt_val;
                break;

            case nbase_token_GREATER:
                node.v.flt_val = node.v.flt_val > node2.v.flt_val;
                break;

            case nbase_token_GREATEREQ:
                node.v.flt_val = node.v.flt_val >= node2.v.flt_val;
                break;

            case nbase_token_EQ:
                node.v.flt_val = node.v.flt_val == node2.v.flt_val;
                break;

            case nbase_token_NEQUALS:
                node.v.flt_val = node.v.flt_val != node2.v.flt_val;
                break;

            default:
                break;
            }
            /* INTEGER ****************************************************** END */
            break;

        case nbase_datatype_INTEGER:
            /* FLOAT ************************************************************ */
            switch(oper)
            {
            case nbase_token_LESS:
                node.v.int_val = node.v.int_val < node2.v.int_val;
                break;

            case nbase_token_LESSEQ:
                node.v.int_val = node.v.int_val <= node2.v.int_val;
                break;

            case nbase_token_GREATER:
                node.v.int_val = node.v.int_val > node2.v.int_val;
                break;

            case nbase_token_GREATEREQ:
                node.v.int_val = node.v.int_val >= node2.v.int_val;
                break;

            case nbase_token_EQ:
                node.v.int_val = node.v.int_val == node2.v.int_val;
                break;

            case nbase_token_NEQUALS:
                node.v.int_val = node.v.int_val != node2.v.int_val;
                break;

            default:
                break;
            }
            /* FLOAT ******************************************************** END */
            break;

        default:
            break;
        }
    }

    *pNodeOut = node;
}

#endif /* EVAL_IMPLEMENTATION */
