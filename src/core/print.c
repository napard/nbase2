/*
 * print.c
 * PRINT
 *
 * 04 nov 2022 -- 17:39 -03
 * Notes:
 * 
 * A real headache to implement variable args keywords...
 * Also, PRINT is one of several cases were the line is tokenized and run in place.
 * For keywords that are not tokenized and run in place (eg. they have no arguments and
 * its handler is simpler than this), the handler is executed when parsed, the default
 * way of compiling (tokenizing as part of a program) such simple commands is through
 * '.' keyword (alias LOADLINE).
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

    /* If not tokenizing, clear code area to evaluate this line now. */
    if(!NBASE_TOKENIZING)
    {
        memset(NBASE_CODE_AREA_BASE, 0, NBASE_MAX_RAM_SIZE - NBASE_MAX_VARS_AREA_SIZE - NBASE_MAX_DATA_AREA_SIZE);
        g_state.code_limit = NBASE_CODE_AREA_BASE;
    }
    
    nbase_tokenize_keyword(nbase_token_PRINT);

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
        nbase_tokenize_keyword(nbase_token_NL);
        nl = false;
    
        if (g_state.next_tok == nbase_token_COLON)
        {
            /* If we get a colon, we expect a next command
                on the same line as this PRINT. */
            
            /* If not tokenizing, run this line tokenized code now. */
            if(!NBASE_TOKENIZING)
            {
                nbase_keyword_RUN();
            }
            
            return;
        }
    }
    else
    {
        /* PRINT with args... */
        while (node)
        {
            nl = true;

            if (g_state.next_tok == nbase_token_COLON)
            {
                /* If we get a colon, we expect a next command
                    on the same line as this PRINT. */
                
                /* If not tokenizing, run this line tokenized code now. */
                if(!NBASE_TOKENIZING)
                {
                    nbase_keyword_RUN();
                }
                
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
            else
                nbase_tokenize_keyword(nbase_token_SEMICOLON);
            
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
        nbase_tokenize_keyword(nbase_token_NL);
    }

    g_state.next_tok = nbase_token_EOL;
    /*nbase_tokenize_keyword(nbase_token_EOL);*/

    /* If not tokenizing, run this line tokenized code now. */
    if(!NBASE_TOKENIZING)
    {
        nbase_keyword_RUN();
    }
}

#endif /* PRINT_IMPLEMENTATION */
