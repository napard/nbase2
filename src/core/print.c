/*
 * print.c
 * PRINT
 *
 * 04 nov 2022 -- 17:39 -03
 * Notes:
 */

/* -------------------------------------------------------------------------------- */
/* Implementation. */

#ifdef PRINT_IMPLEMENTATION
#undef PRINT_IMPLEMENTATION

static const char* PRINT_FILE = "print.c";

/* ******************************************************************************** */

/* PRINT */
void nbase_keyword_PRINT()
{
    nbase_ast_node* node = NULL;
    NBASE_BOOL nl = true;

    NBASE_BOOL force_not_null = false;
    node = (nbase_ast_node*)nbase_parse_expression(&force_not_null);

#ifdef NBASE_DEBUG__
    NBASE_PRINT("/* PRINT\n");
    nbase_print_ast_node(node, NULL);
    NBASE_PRINT("*/\n");
#endif /* NBASE_DEBUG */

    if(!node)
    {
        /* PRINT without args... */
        NBASE_PRINT("\n");
        nl = false;
    }
    else
    {
        /* PRINT with args... */
        while (node)
        {
            nl = true;
            
            node = (nbase_ast_node*)nbase_eval_expression((NBASE_OBJECT*)node);
            
            switch(node->ast_type)
            {
            case nbase_ast_type_FACTOR:
                switch(node->data_type)
                {
                case nbase_datatype_FLOAT:
                    NBASE_PRINTF("%.50f", node->u.flt_val);
                    break;
                    
                case nbase_datatype_INTEGER:
                    NBASE_PRINTF("%d", node->u.int_val);
                    break;

                case nbase_datatype_STRING:
                    NBASE_PRINTF("%s", node->u.str_val);
                    break;

                default:
                    NBASE_ASSERT_OR_INTERNAL_ERROR(0,
                        "UNKNOWN DATA TYPE FOR AST NODE", PRINT_FILE, __FUNCTION__, __LINE__);
                }
                break;
                
            default:
                NBASE_ASSERT_OR_INTERNAL_ERROR(0,
                    "UNKNOWN NODE TYPE", PRINT_FILE, __FUNCTION__, __LINE__);
            }            
            
            if (g_state.next_tok == nbase_token_COLON)
            {
                /* If we get a colon, we expect a next command
                    on the same line as this PRINT. */
                return;
            }
            else if (g_state.next_tok != nbase_token_SEMICOLON)
            {
                /* If we got other than colon or semicolon, asume
                    garbage at end of line. */
                NBASE_ASSERT_OR_ERROR(g_state.next_tok == nbase_token_EOL,
                    nbase_error_type_EXPECTED_END_OF_LINE, PRINT_FILE, __FUNCTION__, __LINE__, "");
                break;
            }
            
            /* We got a semicolon, if next node parsed is not NULL, print it,
                if it is NULL, leave the loop and a newline WILL NOT be printed at the end. */
            
            nl = false;
            force_not_null = false;
            node = (nbase_ast_node*)nbase_parse_expression(&force_not_null);
#ifdef NBASE_DEBUG__            
            NBASE_PRINT("/* PRINT\n");
            nbase_print_ast_node(node, NULL);
            NBASE_PRINT("*/\n");
#endif /* NBASE_DEBUG */
        }
    }

    if (nl)
    {
        NBASE_PRINT("\n");
    }

    g_state.next_tok = nbase_token_EOL;
}

#endif /* PRINT_IMPLEMENTATION */
