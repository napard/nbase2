/*
 * vars.c
 * 
 *
 * 13 sep 2022 -- 08:23 -03
 * Notes:
 */

#ifdef VARS_DEFINITIONS
#undef VARS_DEFINITIONS

/* -------------------------------------------------------------------------------- */
/* Type definitions. */

/*! Variable structure. */
typedef struct _nbase_variable
{
    NBASE_INTEGER data_offset;
    NBASE_DIMENSION dims[NBASE_MAX_ARRAY_DIMENSION];
    char name[NBASE_MAX_VAR_NAME_LEN + 1];
    nbase_datatype type;
} nbase_variable;

/* -------------------------------------------------------------------------------- */
/* Prototypes. */

void nbase_search_variable              (const char* pName, nbase_variable* pVarData);
void nbase_add_variable                 (const char* pName, nbase_datatype pType, uint8_t* pArrayDim);

#endif /* VARS_DEFINITIONS */

/* -------------------------------------------------------------------------------- */
/* Implementation. */

#ifdef VARS_IMPLEMENTATION
#undef VARS_IMPLEMENTATION

/* ******************************************************************************** */

void nbase_search_variable(const char* pName, nbase_variable* pVarData)
{
    uint64_t* p = (uint64_t*)NBASE_VARS_AREA_BASE;
    pVarData->type = nbase_datatype_NONE;
    memset(pVarData->dims, 0, NBASE_MAX_ARRAY_DIMENSION * sizeof(NBASE_DIMENSION));
    
    /* Search in vars descriptor area a match in the name. */
    while(p < g_state.vars_limit)
    {
        if(!strncmp(pName, (const char*)p + 3, /*strlen((const char*)p + 3)*/5))
        {
            /* Found, get var data offset in data area. */
            strcpy(pVarData->name, pName);
            pVarData->data_offset = NBASE_VARDATA_OFFSET(*p);
            
            /* Get type. */
            switch(pVarData->name[strlen(pName) - 1])
            {
            case '%':
                pVarData->type = nbase_datatype_INTEGER;
                break;
            case '$':
                pVarData->type = nbase_datatype_STRING;
                break;
            
            default:
                pVarData->type = nbase_datatype_FLOAT;
            }
            break;
        }
        
        p++;
    }
}

/* ******************************************************************************** */

void nbase_add_variable(const char* pName, nbase_datatype pType, uint8_t* pArrayDim)
{
    /* Start of var descriptor. */
    uint8_t* var_offset = (uint8_t*)g_state.vars_limit;
    uint64_t* v = (uint64_t*)var_offset;

    /* Copy name, including type sentinel. */
    strncpy((char*)var_offset + 3, pName, strlen(pName));
    /* Pad with zeros up to 5 bytes. */
    memset(var_offset + 3 + strlen(pName), 0, 5 - strlen(pName));
    
    /* Mark as object if variable is a string. */
    if(pType == nbase_datatype_STRING)
    {
        *v |= NBASE_OBJECTBIT_MASK;
    }
    
    /* Set offset. */
    *v |= NBASE_VARDATA_OFFSET(g_state.data_limit - NBASE_DATA_AREA_BASE);

    /* Update vars descriptors limit. */
    g_state.vars_limit++;

    /* Calculate total data limit and update it. */
    if(pArrayDim && pArrayDim[0] > 0)
    {
        uint32_t total_dim = 0, i;
        for(i = 0; i < NBASE_MAX_ARRAY_DIMENSION; i++)
        {
            total_dim += pArrayDim[i] * nbase_get_size_of_type(pType);
        }
        
        g_state.data_limit += total_dim;
    }
    else
    {
        g_state.data_limit += nbase_get_size_of_type(pType);
    }
}

#endif /* VARS_IMPLEMENTATION */
