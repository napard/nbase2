/*
 * keywords.c
 * Core keywords.
 *
 * 03 nov 2022 -- 12:10 -03
 * Notes:
 */

/* -------------------------------------------------------------------------------- */
/* Implementation. */

#ifdef KEYWORDS_IMPLEMENTATION
#undef KEYWORDS_IMPLEMENTATION

const char* KEYWORDS_FILE = "keywords.c";

/* ******************************************************************************** */

/* END */
void nbase_keyword_END()
{
    if(!NBASE_TOKENIZING)
    {
        nbase_gc();
        exit(0);
    }

    nbase_tokenize_keyword(nbase_token_END);
    /* Parse ending colon character if there is any. */
    nbase_parse_colon();
}

/* ******************************************************************************** */

/* . */
void nbase_keyword_LOADLINE()
{
#ifdef NBASE_STANDALONE
    NBASE_TRY
    {
        g_state.state_flags |= nbase_state_flag_TOKENIZING;
        
        g_state.nextchar = g_state.input_buffer + 1;
cont2:        
        g_state.next_tok = (nbase_token)nbase_get_next_token(true, false);
        
        if(g_state.next_tok == nbase_token_EOL)
        {
            if(NBASE_TOKENIZING)
            {
                nbase_tokenize_keyword(nbase_token_EOL);
            }
            else
            {
                g_state.in_line++;
            } /* NBASE_TOKENIZING */

            g_state.state_flags &= ~nbase_state_flag_TOKENIZING;
            return;
        }
        else if(g_state.next_tok == nbase_token_COLON)
            goto cont2;
    }
    NBASE_CATCH
    {
        nbase_reset_state();
    }
    NBASE_ENDTRY;

    nbase_tokenize_keyword(nbase_token_EOL);
    g_state.state_flags &= ~nbase_state_flag_TOKENIZING;
#endif /* NBASE_STANDALONE */
}

/* ******************************************************************************** */

/* GC */
void nbase_keyword_GC()
{
    if(!NBASE_TOKENIZING)
    {
        nbase_gc();
        /* Parse ending colon character if there is any. */
        nbase_parse_colon();
        return;
    }

    nbase_tokenize_keyword(nbase_token_GC);
    /* Parse ending colon character if there is any. */
    nbase_parse_colon();
}

#endif /* KEYWORDS_IMPLEMENTATION */
