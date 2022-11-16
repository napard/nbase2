/*
 * dump.c
 * DUMP
 *
 * 01 nov 2022 -- 18:53 -03
 * Notes:
 */

/* -------------------------------------------------------------------------------- */
/* Implementation. */

#ifdef DEBUGTOOLS_DUMP_IMPLEMENTATION
#undef DEBUGTOOLS_DUMP_IMPLEMENTATION

/* ******************************************************************************** */

/* DUMP */
void nbase_keyword_DUMP()
{
    if(!NBASE_TOKENIZING)
    {
        uint32_t i, k = 0;
        uint8_t* p = NBASE_CODE_AREA_BASE;
        for(i = 0; i < 128; i++)
        {
            NBASE_PRINTF("%02x ", *(p + i) & 0xff);
            if(i > 0 && ((i + 1) % 4) == 0)
            {
                NBASE_PRINTF("- ");
            }
            if(i > 0 && ((i + 1) % 16) == 0)
            {
                for(; k <= i; k++)
                {
                    if(*(p + k) >= 32 && *(p + k) <= 126)
                    {
                        NBASE_PRINTF("%c", *(p + k));
                    }
                    else
                    {
                        NBASE_PRINTF(".");
                    }
                }
                NBASE_PRINTF("\n");
            }
        }
    
        /* Parse ending colon character if there is any. */
        nbase_parse_colon();
        return;
    }

    nbase_tokenize_keyword(nbase_token_DUMP);
    /* Parse ending colon character if there is any. */
    nbase_parse_colon();
}

#endif /* DEBUGTOOLS_DUMP_IMPLEMENTATION */
