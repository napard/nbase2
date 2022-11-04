/*
 * run.c
 * RUN
 *
 * 04 nov 2022 -- 11:28 -03
 * Notes:
 */

/* -------------------------------------------------------------------------------- */
/* Implementation. */

#ifdef RUN_IMPLEMENTATION
#undef RUN_IMPLEMENTATION

static const char* RUN_FILE = "run.c";

/* ******************************************************************************** */

/* RUN */
void nbase_keyword_RUN()
{
    uint16_t* pcode = (uint16_t*)NBASE_CODE_AREA_BASE;

    switch(*pcode)
    {
    case nbase_token_END:           nbase_keyword_END(); break;

    default:
        NBASE_ASSERT_OR_ERROR(0,
            nbase_error_type_unknown_TOKEN, RUN_FILE, __FUNCTION__, __LINE__, "");
    }
}

#endif /* RUN_IMPLEMENTATION */
