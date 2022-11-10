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
/* Prototypes. */

NBASE_OBJECT*                           nbase_eval_unary(NBASE_OBJECT* pNode, uint16_t pOper);
NBASE_OBJECT*                           nbase_eval_binary(NBASE_OBJECT* pLhs, NBASE_OBJECT* pRhs, uint16_t pOper);
NBASE_OBJECT*                           nbase_eval_expression(NBASE_OBJECT* pNode);

#endif /* EVAL_DEFINITIONS */
 
/* -------------------------------------------------------------------------------- */
/* Implementation. */

#ifdef EVAL_IMPLEMENTATION
#undef EVAL_IMPLEMENTATION

const char* EVAL_FILE = "eval.c";

NBASE_OBJECT* nbase_eval_unary(NBASE_OBJECT* pNode, uint16_t pOper)
{
#if 0
    nbase_ast_node* node_arg = (nbase_ast_node*)pNode;
    switch(pOper)
    {
    /* '%' ********************************************************** */
    case nbase_token_NEGATED:
    {
        switch(node_arg->ast_type)
        {
        case nbase_ast_type_FACTOR:
            switch(node_arg->data_type)
            {
            case nbase_datatype_FLOAT:
                pNode = nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_FLOAT, NULL,
                    0, -node_arg->u.flt_val, NULL, NULL, 0, NULL);
                break;
            
            case nbase_datatype_INTEGER_:
                pNode = nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_INTEGER_, NULL,
                    -node_arg->u.int_val, 0, NULL, NULL, 0, NULL);
                break;

            default:
                NBASE_ASSERT_OR_INTERNAL_ERROR(0,
                    "UNKNOWN DATA TYPE FOR AST NODE", EVAL_FILE, __LINE__);
            }
            break;

        case nbase_ast_type_UNARY:
        {
            nbase_ast_node* node = (nbase_ast_node*)nbase_eval_unary(node_arg->u.op.lhs, node_arg->u.op.oper);
            switch(node->data_type)
            {
            case nbase_datatype_FLOAT:
                return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_FLOAT, NULL,
                    0, -node->u.flt_val, NULL, NULL, 0, NULL);
                break;
            
            case nbase_datatype_INTEGER_:
                return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_INTEGER_, NULL,
                    -node->u.int_val, 0, NULL, NULL, 0, NULL);
                break;
            
            default:
                NBASE_ASSERT_OR_INTERNAL_ERROR(0,
                    "UNKNOWN DATA TYPE FOR AST NODE", EVAL_FILE, __LINE__);
            }
        }
            break;
        
        case nbase_ast_type_BINARY:
        {
            nbase_ast_node* node = (nbase_ast_node*)nbase_eval_binary(node_arg->u.op.lhs, node_arg->u.op.rhs, node_arg->u.op.oper);
            switch(node->data_type)
            {
            case nbase_datatype_FLOAT:
                return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_FLOAT, NULL,
                    0, -node->u.flt_val, NULL, NULL, 0, NULL);
                break;

            case nbase_datatype_INTEGER_:
                return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_INTEGER_, NULL,
                    -node->u.int_val, 0, NULL, NULL, 0, NULL);
                break;

            default:
                NBASE_ASSERT_OR_INTERNAL_ERROR(0,
                    "UNKNOWN DATA TYPE FOR AST NODE", EVAL_FILE, __LINE__);
            }
        }
            break;

        case nbase_ast_type_VARIABLE:
        {
            nbase_ast_node* node = (nbase_ast_node*)nbase_eval_expression((NBASE_OBJECT*)node_arg);
            switch(node->data_type)
            {
            case nbase_datatype_FLOAT:
                node->u.flt_val = -node->u.flt_val;
                return (NBASE_OBJECT*)node;
                break;
            
            case nbase_datatype_INTEGER_:
                node->u.int_val = -node->u.int_val;
                return (NBASE_OBJECT*)node;
                break;

            default:
                NBASE_ASSERT_OR_INTERNAL_ERROR(0,
                    "UNKNOWN DATA TYPE FOR AST NODE", EVAL_FILE, __LINE__);
            }
        }
            break;

        default:
            NBASE_ASSERT_OR_INTERNAL_ERROR(0,
                "UNKNOWN NODE TYPE", EVAL_FILE, __LINE__);
        }
    }
        break;
    /* '%' ********************************************************** END */

    /* 'NOT' ********************************************************** */
    case nbase_token_NOT:
    {
        switch(node_arg->ast_type)
        {
        case nbase_ast_type_FACTOR:
            switch(node_arg->data_type)
            {
            case nbase_datatype_FLOAT:
                return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_FLOAT, NULL,
                    0, ! node_arg->u.flt_val, NULL, NULL, 0, NULL);
                break;
            
            case nbase_datatype_INTEGER_:
                return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_INTEGER_, NULL,
                    ! node_arg->u.int_val, 0, NULL, NULL, 0, NULL);
                break;

            default:
                NBASE_ASSERT_OR_INTERNAL_ERROR(0,
                    "UNKNOWN DATA TYPE FOR AST NODE", EVAL_FILE, __LINE__);
            }
            break;

        case nbase_ast_type_UNARY:
        {
            nbase_ast_node* node = (nbase_ast_node*)nbase_eval_unary(node_arg->u.op.lhs, node_arg->u.op.oper);
            switch(node->data_type)
            {
            case nbase_datatype_FLOAT:
                return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_FLOAT, NULL,
                    0, ! node->u.flt_val, NULL, NULL, 0, NULL);
                break;

            case nbase_datatype_INTEGER_:
                return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_INTEGER_, NULL,
                    ! node->u.int_val, 0, NULL, NULL, 0, NULL);
                break;

            default:
                NBASE_ASSERT_OR_INTERNAL_ERROR(0,
                    "UNKNOWN DATA TYPE FOR AST NODE", EVAL_FILE, __LINE__);
            }
        }
            break;
        
        case nbase_ast_type_BINARY:
        {
            nbase_ast_node* node = (nbase_ast_node*)nbase_eval_binary(node_arg->u.op.lhs, node_arg->u.op.rhs, node_arg->u.op.oper);
            switch(node->data_type)
            {
            case nbase_datatype_FLOAT:
                return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_FLOAT, NULL,
                    0, ! node->u.flt_val, NULL, NULL, 0, NULL);
                break;
            
            case nbase_datatype_INTEGER_:
                return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_INTEGER_, NULL,
                    ! node->u.int_val, 0, NULL, NULL, 0, NULL);
                break;

            default:
                NBASE_ASSERT_OR_INTERNAL_ERROR(0,
                    "UNKNOWN DATA TYPE FOR AST NODE", EVAL_FILE, __LINE__);
            }
        }
            break;

        case nbase_ast_type_VARIABLE:
        {
            nbase_ast_node* node = (nbase_ast_node*)nbase_eval_expression((NBASE_OBJECT*)node_arg);
            switch(node->data_type)
            {
            case nbase_datatype_FLOAT:
                node->u.flt_val = !node->u.flt_val;
                return (NBASE_OBJECT*)node;
                break;
            
            case nbase_datatype_INTEGER_:
                node->u.int_val = !node->u.int_val;
                return (NBASE_OBJECT*)node;
                break;

            default:
                NBASE_ASSERT_OR_INTERNAL_ERROR(0,
                    "UNKNOWN DATA TYPE FOR AST NODE", EVAL_FILE, __LINE__);
            }
        }
            break;

        default:
            NBASE_ASSERT_OR_INTERNAL_ERROR(0,
                "UNKNOWN NODE TYPE", EVAL_FILE, __LINE__);
        }
    }
        break;
    /* 'NOT' ********************************************************** END */

    default:
        NBASE_ASSERT_OR_INTERNAL_ERROR(0,
            "UNKNOWN UNARY OPERATOR", EVAL_FILE, __LINE__);
    }

    return pNode;
#endif
}

NBASE_OBJECT* nbase_eval_binary_(NBASE_OBJECT* pLhs, NBASE_OBJECT* pRhs, uint16_t pOper)
{
#if 0

#if 0
    if(NBASE_AST_NODE(pLhs)->ast_type == nbase_ast_type_UNARY)
        pLhs = eval_unary(NBASE_AST_NODE(pLhs)->u.op.lhs, NBASE_AST_NODE(pLhs)->u.op.oper);
    if(NBASE_AST_NODE(pLhs)->ast_type == nbase_ast_type_BINARY)
        pLhs = eval_binary(NBASE_AST_NODE(pLhs)->u.op.lhs, NBASE_AST_NODE(pLhs)->u.op.rhs, NBASE_AST_NODE(pLhs)->u.op.oper);

    if(NBASE_AST_NODE(pRhs)->ast_type == nbase_ast_type_UNARY)
        pRhs = eval_unary(NBASE_AST_NODE(pRhs)->u.op.lhs, NBASE_AST_NODE(pRhs)->u.op.oper);
    if(NBASE_AST_NODE(pRhs)->ast_type == nbase_ast_type_BINARY)
        pRhs = eval_binary(NBASE_AST_NODE(pRhs)->u.op.lhs, NBASE_AST_NODE(pRhs)->u.op.rhs, NBASE_AST_NODE(pRhs)->u.op.oper);
#endif

    pLhs = nbase_eval_expression(pLhs);
    pRhs = nbase_eval_expression(pRhs);

    switch(NBASE_AST_NODE(pLhs)->data_type)
    {
    /* INTEGER ********************************************************** */
    case nbase_datatype_INTEGER_:
        switch(pOper)
        {
        case nbase_token_PLUS:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_INTEGER_, NULL,
                NBASE_AST_NODE(pLhs)->u.int_val + NBASE_AST_NODE(pRhs)->u.int_val, 0, NULL, NULL, 0, NULL);
            break;
        case nbase_token_MINUS:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_INTEGER_, NULL,
                NBASE_AST_NODE(pLhs)->u.int_val - NBASE_AST_NODE(pRhs)->u.int_val, 0, NULL, NULL, 0, NULL);
            break;
        case nbase_token_MUL:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_INTEGER_, NULL,
                NBASE_AST_NODE(pLhs)->u.int_val * NBASE_AST_NODE(pRhs)->u.int_val, 0, NULL, NULL, 0, NULL);
            break;
        case nbase_token_DIV:
            if(NBASE_AST_NODE(pRhs)->u.int_val == 0)
                nbase_error(nbase_error_type_DIVISION_BY_ZERO, THIS_FILE, __LINE__, "");
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_INTEGER_, NULL,
                NBASE_AST_NODE(pLhs)->u.int_val / NBASE_AST_NODE(pRhs)->u.int_val, 0, NULL, NULL, 0, NULL);
            break;
        case nbase_token_MOD:
            if(NBASE_AST_NODE(pRhs)->u.int_val == 0)
                nbase_error(nbase_error_type_DIVISION_BY_ZERO, THIS_FILE, __LINE__, "");
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_INTEGER_, NULL,
                NBASE_AST_NODE(pLhs)->u.int_val % NBASE_AST_NODE(pRhs)->u.int_val, 0, NULL, NULL, 0, NULL);
            break;
        case nbase_token_POW:
        {
            double res;
            errno = 0; feclearexcept(FE_ALL_EXCEPT);
            res = pow(NBASE_AST_NODE(pLhs)->u.int_val, NBASE_AST_NODE(pRhs)->u.int_val);
            if(errno == EDOM) perror("    errno == EDOM");
            if(fetestexcept(FE_INVALID))
                nbase_error(nbase_error_type_DOM_ERROR_1, THIS_FILE, __LINE__, "");
            if(fetestexcept(FE_DIVBYZERO))
                nbase_error(nbase_error_type_DOM_ERROR_2, THIS_FILE, __LINE__, "");
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_INTEGER_, NULL,
                res, 0, NULL, NULL, 0, NULL);
        }
            break;
        case nbase_token_LSHIFT:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_INTEGER_, NULL,
                NBASE_AST_NODE(pLhs)->u.int_val << NBASE_AST_NODE(pRhs)->u.int_val, 0, NULL, NULL, 0, NULL);
            break;
        case nbase_token_RSHIFT:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_INTEGER_, NULL,
                NBASE_AST_NODE(pLhs)->u.int_val >> NBASE_AST_NODE(pRhs)->u.int_val, 0, NULL, NULL, 0, NULL);
            break;
        case nbase_token_LESS:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_INTEGER_, NULL,
                -(NBASE_AST_NODE(pLhs)->u.int_val < NBASE_AST_NODE(pRhs)->u.int_val), 0, NULL, NULL, 0, NULL);
            break;
        case nbase_token_LESSEQ:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_INTEGER_, NULL,
                -(NBASE_AST_NODE(pLhs)->u.int_val <= NBASE_AST_NODE(pRhs)->u.int_val), 0, NULL, NULL, 0, NULL);
            break;
        case nbase_token_GREATER:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_INTEGER_, NULL,
                -(NBASE_AST_NODE(pLhs)->u.int_val > NBASE_AST_NODE(pRhs)->u.int_val), 0, NULL, NULL, 0, NULL);
            break;
        case nbase_token_GREATEREQ:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_INTEGER_, NULL,
                -(NBASE_AST_NODE(pLhs)->u.int_val >= NBASE_AST_NODE(pRhs)->u.int_val), 0, NULL, NULL, 0, NULL);
            break;
        case nbase_token_EQ:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_INTEGER_, NULL,
                -(NBASE_AST_NODE(pLhs)->u.int_val == NBASE_AST_NODE(pRhs)->u.int_val), 0, NULL, NULL, 0, NULL);
            break;
        case nbase_token_NEQUALS:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_INTEGER_, NULL,
                -(NBASE_AST_NODE(pLhs)->u.int_val != NBASE_AST_NODE(pRhs)->u.int_val), 0, NULL, NULL, 0, NULL);
            break;
        case nbase_token_AND:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_INTEGER_, NULL,
                NBASE_AST_NODE(pLhs)->u.int_val & NBASE_AST_NODE(pRhs)->u.int_val, 0, NULL, NULL, 0, NULL);
            break;
        case nbase_token_OR:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_INTEGER_, NULL,
                NBASE_AST_NODE(pLhs)->u.int_val | NBASE_AST_NODE(pRhs)->u.int_val, 0, NULL, NULL, 0, NULL);
            break;
        case nbase_token_XOR:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_INTEGER_, NULL,
                NBASE_AST_NODE(pLhs)->u.int_val ^ NBASE_AST_NODE(pRhs)->u.int_val, 0, NULL, NULL, 0, NULL);
            break;

        default:
            NBASE_ASSERT_OR_INTERNAL_ERROR(0,
                "UNKNOWN BINARY OPERATOR", EVAL_FILE, __LINE__);
        }
        break;
    /* INTEGER ********************************************************** END */
    
    /* FLOAT ********************************************************** */
    case nbase_datatype_FLOAT:
        switch(pOper)
        {
        case nbase_token_PLUS:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_FLOAT, NULL,
                0, NBASE_AST_NODE(pLhs)->u.flt_val + NBASE_AST_NODE(pRhs)->u.flt_val, NULL, NULL, 0, NULL);
            break;
        case nbase_token_MINUS:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_FLOAT, NULL,
                0, NBASE_AST_NODE(pLhs)->u.flt_val - NBASE_AST_NODE(pRhs)->u.flt_val, NULL, NULL, 0, NULL);
            break;
        case nbase_token_MUL:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_FLOAT, NULL,
                0, NBASE_AST_NODE(pLhs)->u.flt_val * NBASE_AST_NODE(pRhs)->u.flt_val, NULL, NULL, 0, NULL);
            break;
        case nbase_token_DIV:
            if(NBASE_AST_NODE(pRhs)->u.flt_val == 0)
                nbase_error(nbase_error_type_DIVISION_BY_ZERO, THIS_FILE, __LINE__, "");
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_FLOAT, NULL,
                0, NBASE_AST_NODE(pLhs)->u.flt_val / NBASE_AST_NODE(pRhs)->u.flt_val, NULL, NULL, 0, NULL);
            break;
        /*case nbase_token_MOD:
            if(NBASE_AST_NODE(pRhs)->u.flt_val == 0)
                error(nbase_error_type_DIVISION_BY_ZERO, THIS_FILE, __LINE__, "");
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_FLOAT, NULL,
                NBASE_AST_NODE(pLhs)->u.flt_val % NBASE_AST_NODE(pRhs)->u.flt_val, NULL, NULL, 0, NULL);
            break;*/
        case nbase_token_POW:
        {
            double res;
            errno = 0; feclearexcept(FE_ALL_EXCEPT);
            res = pow(NBASE_AST_NODE(pLhs)->u.flt_val, NBASE_AST_NODE(pRhs)->u.flt_val);
            if(errno == EDOM) perror("    errno == EDOM");
            if(fetestexcept(FE_INVALID))
                nbase_error(nbase_error_type_DOM_ERROR_1, THIS_FILE, __LINE__, "");
            if(fetestexcept(FE_DIVBYZERO))
                nbase_error(nbase_error_type_DOM_ERROR_2, THIS_FILE, __LINE__, "");
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_FLOAT, NULL,
                0, res, NULL, NULL, 0, NULL);
        }
            break;
        /*case nbase_token_LSHIFT:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_FLOAT, NULL,
                NBASE_AST_NODE(pLhs)->u.int_val << NBASE_AST_NODE(pRhs)->u.int_val, 0, NULL, NULL, 0, NULL);
            break;
        case nbase_token_RSHIFT:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_FLOAT, NULL,
                NBASE_AST_NODE(pLhs)->u.int_val >> NBASE_AST_NODE(pRhs)->u.int_val, 0, NULL, NULL, 0, NULL);
            break;*/
        case nbase_token_LESS:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_FLOAT, NULL,
                0, -(NBASE_AST_NODE(pLhs)->u.flt_val < NBASE_AST_NODE(pRhs)->u.flt_val), NULL, NULL, 0, NULL);
            break;
        case nbase_token_LESSEQ:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_FLOAT, NULL,
                0, -(NBASE_AST_NODE(pLhs)->u.flt_val <= NBASE_AST_NODE(pRhs)->u.flt_val), NULL, NULL, 0, NULL);
            break;
        case nbase_token_GREATER:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_FLOAT, NULL,
                0, -(NBASE_AST_NODE(pLhs)->u.flt_val > NBASE_AST_NODE(pRhs)->u.flt_val), NULL, NULL, 0, NULL);
            break;
        case nbase_token_GREATEREQ:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_FLOAT, NULL,
                0, -(NBASE_AST_NODE(pLhs)->u.flt_val >= NBASE_AST_NODE(pRhs)->u.flt_val), NULL, NULL, 0, NULL);
            break;
        case nbase_token_EQ:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_FLOAT, NULL,
                0, -(NBASE_AST_NODE(pLhs)->u.flt_val == NBASE_AST_NODE(pRhs)->u.flt_val), NULL, NULL, 0, NULL);
            break;
        case nbase_token_NEQUALS:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_FLOAT, NULL,
                0, -(NBASE_AST_NODE(pLhs)->u.flt_val != NBASE_AST_NODE(pRhs)->u.flt_val), NULL, NULL, 0, NULL);
            break;
        /*case nbase_token_AND:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_FLOAT, NULL,
                NBASE_AST_NODE(pLhs)->u.int_val & NBASE_AST_NODE(pRhs)->u.int_val, 0, NULL, NULL, 0, NULL);
            break;
        case nbase_token_OR:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_FLOAT, NULL,
                NBASE_AST_NODE(pLhs)->u.int_val | NBASE_AST_NODE(pRhs)->u.int_val, 0, NULL, NULL, 0, NULL);
            break;
        case nbase_token_XOR:
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_FLOAT, NULL,
                NBASE_AST_NODE(pLhs)->u.int_val ^ NBASE_AST_NODE(pRhs)->u.int_val, 0, NULL, NULL, 0, NULL);
            break;*/

        default:
            NBASE_ASSERT_OR_INTERNAL_ERROR(0,
                "UNKNOWN BINARY OPERATOR", EVAL_FILE, __LINE__);
        }
        break;
    /* FLOAT ********************************************************** END */
    
    default:
        NBASE_ASSERT_OR_INTERNAL_ERROR(0,
            "UNKNOWN DATA TYPE!!!", EVAL_FILE, __LINE__);
    }

    return NULL;
#endif
}

NBASE_OBJECT* nbase_eval_binary(NBASE_OBJECT* pLhs, NBASE_OBJECT* pRhs, uint16_t pOper)
{
#if 0
#ifdef DEBUG__
    NBASE_PRINT("/* eval_binary()\n");
    for(int32_t i = 0; i < g_tab_level1; i++)
        NBASE_PRINTF("  ");

    NBASE_PRINTF("EVAL BINARY: oper= %s\n", get_oper_name(pOper));
    g_tab_level1++;
    nbase_print_ast_node((nbase_ast_node*)pLhs, "LHS");
    nbase_print_ast_node((nbase_ast_node*)pRhs, "RHS");
    g_tab_level1--;
    NBASE_PRINT("*/\n");
#endif /* DEBUG */

    switch(pOper)
    {
    case nbase_token_PLUS:
    case nbase_token_MINUS:
    case nbase_token_MUL:
    case nbase_token_DIV:
    case nbase_token_MOD:
    case nbase_token_POW:

    case nbase_token_LSHIFT:
    case nbase_token_RSHIFT:
    case nbase_token_LESS:
    case nbase_token_LESSEQ:
    case nbase_token_GREATER:
    case nbase_token_GREATEREQ:
    case nbase_token_EQ:
    case nbase_token_NEQUALS:
    
    case nbase_token_AND:
    case nbase_token_OR:
    case nbase_token_XOR:
    case nbase_token_NOT:
        if(NBASE_TOKENIZING)
        {
            nbase_eval_expression(pLhs);
            nbase_eval_expression(pRhs);
            nbase_tokenize_binary_op(pLhs, pOper);
        }
        else
        {
            return nbase_eval_binary_(pLhs, pRhs, pOper);
        } /* NBASE_TOKENIZING */
        break;

    default:
        NBASE_ASSERT_OR_INTERNAL_ERROR(0,
            "UNKNOWN BINARY OPERATOR", EVAL_FILE, __LINE__);
    }

    return NULL;
#endif
}

NBASE_OBJECT* nbase_eval_expression(NBASE_OBJECT* pNode)
{
#if 0
    nbase_variable* var;
    
    if(!pNode)
        return NULL;

    switch(NBASE_AST_NODE(pNode)->ast_type)
    {
    case nbase_ast_type_FACTOR:
        if(NBASE_TOKENIZING)
        {
            nbase_tokenize_factor(NBASE_AST_NODE(pNode));
        }
        else
        {
            return pNode;
        } /* NBASE_TOKENIZING */
        break;

    case nbase_ast_type_BINARY:
        pNode = nbase_eval_binary(NBASE_AST_NODE(pNode)->u.op.lhs, NBASE_AST_NODE(pNode)->u.op.rhs, NBASE_AST_NODE(pNode)->u.op.oper);
        break;

    case nbase_ast_type_UNARY:
        if(NBASE_TOKENIZING)
        {
            nbase_tokenize_unary_op(NBASE_AST_NODE(pNode)->u.op.lhs, NBASE_AST_NODE(pNode)->u.op.oper);
        }
        else
        {
            pNode = nbase_eval_unary(NBASE_AST_NODE(pNode)->u.op.lhs, NBASE_AST_NODE(pNode)->u.op.oper);
        } /* NBASE_TOKENIZING */
        break;
    
    /* TODO */
    case nbase_ast_type_VARIABLE:
        switch(NBASE_AST_NODE(pNode)->data_type)
        {
        case nbase_datatype_INTEGER_:
        {
            int32_t dim_sum = 0;
            var = NBASE_AST_NODE(pNode)->extra;
            if(var && var->dims && var->dims[0] > 0)
            {
                uint32_t i;
                for(i = 0; i < NBASE_MAX_ARRAY_DIMENSION; i++)
                    dim_sum += var->dims[i];
            }
            else
                dim_sum = 1;
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_INTEGER_,
                NULL, *((NBASE_INTEGER*)(var->data_offset + (dim_sum - 1) * nbase_get_size_of_type(var->type) + NBASE_DATA_AREA_BASE)), 0, NULL, NULL, 0, NULL);
        }
            break;

        case nbase_datatype_FLOAT:
        {
            int32_t dim_sum = 0;
            var = NBASE_AST_NODE(pNode)->extra;
            if(var && var->dims && var->dims[0] > 0)
            {
                uint32_t i;
                for(i = 0; i < NBASE_MAX_ARRAY_DIMENSION; i++)
                    dim_sum += var->dims[i];
            }
            else
                dim_sum = 1;
            return nbase_alloc_ast_node(nbase_ast_type_FACTOR, nbase_datatype_FLOAT,
                NULL, 0, *((NBASE_FLOAT*)(var->data_offset + (dim_sum - 1) * nbase_get_size_of_type(var->type) + NBASE_DATA_AREA_BASE)), NULL, NULL, 0, NULL);
        }
            break;

        case nbase_datatype_STRING:
        {
            int32_t dim_sum = 0;
            var = NBASE_AST_NODE(pNode)->extra;
            if(var && var->dims && var->dims[0] > 0)
            {
                uint32_t i;
                for(i = 0; i < NBASE_MAX_ARRAY_DIMENSION; i++)
                    dim_sum += var->dims[i];
            }
            else
                dim_sum = 1;
            /*printf("GET NODE: %p, var->data_offset: %d\n",
                *((NBASE_OBJECT**)(var->data_offset + (dim_sum - 1) * get_size_of_type(var->type) + NBASE_DATA_AREA_BASE)),
                var->data_offset);*/
            return *((NBASE_OBJECT**)(var->data_offset + (dim_sum - 1) * nbase_get_size_of_type(var->type) + NBASE_DATA_AREA_BASE));
        }
            break;
        
        default:
            NBASE_ASSERT_OR_INTERNAL_ERROR(0,
                "UNKNOWN DATA TYPE", EVAL_FILE, __LINE__);
        }
        break;
    
    default:
        NBASE_ASSERT_OR_INTERNAL_ERROR(0,
            "UNKNOWN NODE TYPE", EVAL_FILE, __LINE__);
    }
    
    return pNode;
#endif
}

#endif /* EVAL_IMPLEMENTATION */
