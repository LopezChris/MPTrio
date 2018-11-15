/*
 *     SocialLedge.com - Copyright (C) 2013
 *
 *     This file is part of free software framework for embedded processors.
 *     You can use it and/or distribute it as long as this copyright header
 *     remains unmodified.  The code is free for personal use and requires
 *     permission to use in a commercial product.
 *
 *      THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 *      OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 *      MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *      I SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 *      CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *     You can reach the author of this software at :
 *          p r e e t . w i k i @ g m a i l . c o m
 */
#include "sys_config.h"
#if ENABLE_TELEMETRY
#include <stdio.h>
#include <stdlib.h> /* malloc() */
#include <string.h> /* strcmp() */
#include "c_tlm_var.h"


/** Private function of this file */
static c_bool tlm_variable_check_dup(void *elm_ptr, void *arg1,
                                     void *arg2, void *arg3)
{
    tlm_reg_var_type *reg_var = elm_ptr;
    tlm_reg_var_type *new_var = arg1;

    return (reg_var->data_ptr != new_var->data_ptr &&
            0 != strcmp(reg_var->name, new_var->name)
            );
}

/** Private function of this file */
static c_bool tlm_component_find_callback(void *elm_ptr, void *arg1, void *arg2, void *arg3)
{
    tlm_reg_var_type *reg_var = elm_ptr;
    const char *name = arg1;

    /* If we locate the name, make copy of the pointer, and return false
     * to stop iterating further through the list
     */
    return (0 != strcmp(name, reg_var->name));
}



c_bool tlm_variable_register(tlm_component *comp_ptr,
                             const char *name,
                             const void *data_ptr,
                             const uint16_t data_size,
                             const uint16_t arr_size,
                             tlm_type type)
{
    if(NULL == comp_ptr || NULL == name || NULL == data_ptr || 0 == data_size) {
        return c_false;
    }

    tlm_reg_var_type *new_var = malloc(sizeof(tlm_reg_var_type));
    if(NULL == new_var) {
        return c_false;
    }

    /* If not an array, a single var still has size of 1 array element
     * This is make it easier to calculate bytes of the variable
     */
    new_var->name = name;
    new_var->data_ptr = data_ptr;
    new_var->elm_size_bytes = data_size;
    new_var->elm_arr_size = 0 == arr_size ? 1 : arr_size;
    new_var->elm_type = type;

    if (!c_list_for_each_elm(comp_ptr->var_list, tlm_variable_check_dup,
                             (void*)new_var, NULL, NULL)) {
        free(new_var);
        return c_false;
    }

    if (!c_list_insert_elm_end(comp_ptr->var_list, new_var)) {
        free(new_var);
        return c_false;
    }

    return c_true;
}

const tlm_reg_var_type* tlm_variable_get_by_name(tlm_component *comp_ptr, const char *name)
{
    tlm_reg_var_type *reg_var = NULL;
    if (NULL != comp_ptr && NULL != name && '\0' != *name) {
        reg_var = c_list_find_elm(comp_ptr->var_list, tlm_component_find_callback,
                                  (void*)name, NULL, NULL);
    }
    return reg_var;
}

const tlm_reg_var_type* tlm_variable_get_by_comp_and_name(const char *comp_name, const char *name)
{
    tlm_reg_var_type *reg_var = NULL;
    tlm_component *comp_ptr = tlm_component_get_by_name(comp_name);

    if (NULL != comp_ptr && NULL != name && '\0' != *name) {
        reg_var = c_list_find_elm(comp_ptr->var_list, tlm_component_find_callback,
                                  (void*)name, NULL, NULL);
    }

    return reg_var;
}

c_bool tlm_variable_set_value(const char *comp_name, const char *name, const char *value)
{
    const tlm_reg_var_type *reg_var = tlm_variable_get_by_comp_and_name(comp_name, name);
    if (NULL == reg_var) {
        return c_false;
    }

    c_bool success = c_false;
    switch(reg_var->elm_type) {
        case tlm_int:
        {
            long int temp = 0;
            if(1 == sscanf(value, "%li", &temp)) {
                memcpy((void*)reg_var->data_ptr, &temp, reg_var->elm_size_bytes);
                success = c_true;
            }
            break;
        }
        case tlm_uint:
        {
            long unsigned temp = 0;
            if(1 == sscanf(value, "%lu", &temp)) {
                memcpy((void*)reg_var->data_ptr, &temp, reg_var->elm_size_bytes);
                success = c_true;
            }
            break;
        }
        case tlm_char:
        {
            char temp = 0;
            if(1 == sscanf(value, "%c", &temp)) {
                memcpy((void*)reg_var->data_ptr, &temp, reg_var->elm_size_bytes);
                success = c_true;
            }
            break;
        }

        case tlm_string:
            memset((void*)reg_var->data_ptr, 0, reg_var->elm_size_bytes);
            memcpy((void*)reg_var->data_ptr, value,
                    strlen(value) < reg_var->elm_size_bytes ? strlen(value) : reg_var->elm_size_bytes - 1);
            success = c_true;
            break;

        case tlm_bit_or_bool:
            memset((void*)reg_var->data_ptr, (0 == strcmp("true", value)), 1);
            success = c_true;
            break;

        case tlm_binary:
            /* XXX Don't know what to do here :( */
            break;

        case tlm_float:
        {
#if SUPPORT_FLOAT_VAR_SET
            float temp = 0;
            if(1 == sscanf(value, "%f", &temp)) {
                memcpy((void*)reg_var->data_ptr, &temp, reg_var->elm_size_bytes);
                success = c_true;
            }
#else
            int sig = 0;
            unsigned int dec = 0;
            if(2 == sscanf(value, "%i.%1u", &sig, &dec)) {
                float num = dec;
                num /= 10;
                num += sig;
                memcpy((void*)reg_var->data_ptr, &num, reg_var->elm_size_bytes);
                success = c_true;
            }
#endif
            break;
        }

        case tlm_double:
        default:
            success = c_false;
            break;
    }

    return success;
}

#if 0
typedef union {
    float var_float;
    double var_double;
    uint32_t var_32bit;
    uint64_t var_64bit;
    uint8_t  bytes[8];
} tlm_var_type;

static c_bool tlm_variable_print_one(void *p_elm, void *p)
{
    tlm_reg_var_type *p_var = p_elm;
    tlm_var_type v = { 0 };
    uint32_t size = p_var->data_size_bytes;
    if(size > sizeof(v.var_64bit)) {
        size = sizeof(v.var_64bit);
    }
    memcpy(&(v.var_64bit), p_var->p_data, size);

    printf("%s : %u bytes : 0x%X\n",
            p_var->name,
            p_var->data_size_bytes,
            v.var_32bit);

    return c_true;
}

void tlm_variables_print(tlm_component *p_comp)
{
    c_list_for_each_elm(p_comp->tlm_var_list, tlm_variable_print_one, NULL);
}
#endif /* Just testing */

#endif
