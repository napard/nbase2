/*
 * lvar.c
 * LVAR
 *
 * 09 oct 2022 -- 21:43 -03
 * Notes:
 */

/* -------------------------------------------------------------------------------- */
/* Implementation. */

#ifdef DEBUGTOOLS_LVAR_IMPLEMENTATION
#undef DEBUGTOOLS_LVAR_IMPLEMENTATION

/* ******************************************************************************** */

/* LVAR */
void nbase_keyword_LVAR()
{
    uint64_t* p = (uint64_t*)NBASE_VARS_AREA_BASE;
    
    while(p < g_state.vars_limit)
    {
        strncpy(g_temp_buff, (const char*)p + 3, 5);
        g_temp_buff[5] = '\0';
        NBASE_PRINTF("%s : (%d) %08lX (%ld)\n", g_temp_buff, (int)(*p & NBASE_OBJECTBIT_MASK) != 0,
            NBASE_VARDATA_OFFSET(*p), NBASE_VARDATA_OFFSET(*p));
        p++;
    }
}

#endif /* DEBUGTOOLS_LVAR_IMPLEMENTATION */
