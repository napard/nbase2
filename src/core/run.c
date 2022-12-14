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

#ifdef NBASE_INCLUDE_FEATURE_DEBUGTOOLS
void                                    nbase_keyword_LVAR();
void                                    nbase_keyword_STAT();
void                                    nbase_keyword_DUMP();
#endif /* NBASE_INCLUDE_FEATURE_DEBUGTOOLS */    

/* ******************************************************************************** */

/* RUN */
void nbase_keyword_RUN()
{
    uint16_t* pcode = (uint16_t*)NBASE_CODE_AREA_BASE;

next:

    switch(*pcode)
    {
    case nbase_token_END:           nbase_keyword_END(); break;
    case nbase_token_GC:            nbase_keyword_GC(); break;

#ifdef NBASE_INCLUDE_FEATURE_DEBUGTOOLS
    case nbase_token_LVAR:          nbase_keyword_LVAR(); break;
    case nbase_token_STAT:          nbase_keyword_STAT(); break;
    case nbase_token_DUMP:          nbase_keyword_DUMP(); break;
#endif /* NBASE_INCLUDE_FEATURE_DEBUGTOOLS */    

    case nbase_token_NL:
        NBASE_PRINT("\n");
        break;
    
    case nbase_token_RIGHTPAREN:
        break;
    
    case nbase_token_EOL:
        break;
    
    case nbase_token_PRINT:
        if(*(pcode + 1) == nbase_token_NL)
        {
            NBASE_PRINT("\n");
            pcode++;
        }
        else
        {
            nbase_eval_node node, node_out;
            uint8_t* cp;

next_print:            

            /* Build node to evaluate. */
            cp = (uint8_t*)(pcode + 1);
            nbase_token unary = nbase_build_node(&cp, &node);

            /* Call expression evaluator. */
            nbase_eval_expression(unary, &node, &cp, &node_out);
            switch(node.data_type)
            {
            case nbase_datatype_FLOAT:
                NBASE_PRINTF("%.50f", node_out.v.flt_val);
                break;
            
            case nbase_datatype_INTEGER:
                NBASE_PRINTF("%d", node_out.v.int_val);
                break;
            
            case nbase_datatype_STRING:
                NBASE_PRINTF("%s", node_out.v.str_val);
                break;
            
            default:
                NBASE_ASSERT_OR_INTERNAL_ERROR(0,
                    "nbase_keyword_RUN(): UNKNOWN DATA TYPE FOR node_out", RUN_FILE, __FUNCTION__, __LINE__);
                break;
            }
            pcode = (uint16_t*)cp;
            /* Got a ';' ? */
            if(*pcode == nbase_token_SEMICOLON)
                goto next_print;
            goto next;
        }
        break;
    
    case 0:
        if(!(*pcode) && !(*(pcode + 1)))
            /* End of program: two consecutive zero tokens. */
            return;

    default:
        NBASE_ASSERT_OR_ERROR(0,
            nbase_error_type_unknown_TOKEN, RUN_FILE, __FUNCTION__, __LINE__, ", %p: 0x%x (%d)",
                pcode, *pcode, *pcode);
    }
    pcode++;
    goto next;
}

#endif /* RUN_IMPLEMENTATION */
