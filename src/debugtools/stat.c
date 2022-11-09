/*
 * stat.c
 * Print general system state.
 *
 * 13 oct 2022 -- 10:15 -03
 * Notes:
 */

/* -------------------------------------------------------------------------------- */
/* Implementation. */

#ifdef DEBUGTOOLS_STAT_IMPLEMENTATION
#undef DEBUGTOOLS_STAT_IMPLEMENTATION

/* ******************************************************************************** */

/* STAT */
void nbase_keyword_STAT()
{
    if(!NBASE_TOKENIZING)
    {
        uint64_t* p;

        NBASE_PRINT("\nSYSTEM:\n");
        NBASE_PRINTF("SIZEOF nbase_state: %lu\n", sizeof(nbase_state));
        NBASE_PRINTF("SIZEOF nbase_object: %lu\n", sizeof(nbase_object));
        NBASE_PRINTF("NBASE_MAX_LINE_LEN: %d\n", NBASE_MAX_LINE_LEN);
        NBASE_PRINTF("NBASE_MAX_LINE_LEN_PLUS_1: %d\n", NBASE_MAX_LINE_LEN_PLUS_1);
        /*NBASE_PRINTF("NBASE_MAX_LINE_LEN_PLUS_2: %d\n", NBASE_MAX_LINE_LEN_PLUS_2);
        NBASE_PRINTF("NBASE_MAX_LINES: %d (x %d)\n", NBASE_MAX_LINES, NBASE_MAX_LINE_LEN_PLUS_2);*/
        NBASE_PRINTF("NBASE_MAX_VARS_AREA_SIZE: %d\n", NBASE_MAX_VARS_AREA_SIZE);
        NBASE_PRINTF("NBASE_MAX_DATA_AREA_SIZE: %d\n", NBASE_MAX_DATA_AREA_SIZE);
        NBASE_PRINTF("NBASE_GC_INITIAL_THRESHOLD: %d\n", NBASE_GC_INITIAL_THRESHOLD);
        NBASE_PRINTF("SIZEOF g_temp_buff: %lu\n", sizeof(g_temp_buff));
        NBASE_PRINTF("NBASE_VARS_AREA_BASE: %p\n", NBASE_VARS_AREA_BASE);
        NBASE_PRINTF("vars_limit:           %p, %lu bytes used\n", (void*)g_state.vars_limit, (uint8_t*)g_state.vars_limit - NBASE_VARS_AREA_BASE);
        NBASE_PRINTF("NBASE_DATA_AREA_BASE: %p\n", NBASE_DATA_AREA_BASE);
        NBASE_PRINTF("data_limit:           %p, %lu bytes used\n", (void*)g_state.data_limit, (uint8_t*)g_state.data_limit - NBASE_DATA_AREA_BASE);
        NBASE_PRINTF("NBASE_CODE_AREA_BASE: %p\n", NBASE_CODE_AREA_BASE);
        NBASE_PRINTF("code_limit:           %p, %lu bytes used\n", (void*)g_state.code_limit, (uint8_t*)g_state.code_limit - NBASE_CODE_AREA_BASE);
        NBASE_PRINTF("num_objs: %d\n", g_state.num_objs);
        NBASE_PRINTF("max_objs: %d\n", g_state.max_objs);
        NBASE_PRINT("---\n");
        NBASE_PRINT("VARIABLES:\n");
        p = (uint64_t*)NBASE_VARS_AREA_BASE;
        
        while(p < g_state.vars_limit)
        {
            strncpy(g_temp_buff, (const char*)p + 3, 5);
            g_temp_buff[5] = '\0';
            NBASE_PRINTF("%s : (%d) %08lX (%ld)\n", g_temp_buff, (int)(*p & NBASE_OBJECTBIT_MASK) != 0,
                NBASE_VARDATA_OFFSET(*p), NBASE_VARDATA_OFFSET(*p));
            p++;
        }
        NBASE_PRINT("---\n");

        /* Parse ending colon character if there is any. */
        nbase_parse_colon();
        return;
    }

    nbase_tokenize_keyword(nbase_token_STAT);
    /* Parse ending colon character if there is any. */
    nbase_parse_colon();
}

#endif /* DEBUGTOOLS_STAT_IMPLEMENTATION */
