#include "parseconfig.h"
#include <string.h>

int
getLen(const char *name) {
    jl_value_t *arr = jl_eval_string("arr");
    return jl_array_len(arr);
}

void
initConfig(const char *path)
{
    jl_eval_string(strcat(strcat("include\"", path),"\")"));
}

// get variable
void
getConfig_str(char *value, const char *name)
{
    jl_value_t* v = jl_eval_string(name);
    strcpy(value, jl_string_ptr(v));
}

void
getConfig_float(float *f, const char *name)
{
    jl_value_t* v = jl_eval_string(name);
    *f = jl_unbox_float32(v);
}

void
getConfig_int(int *i, const char *name)
{
    jl_value_t* v = jl_eval_string(name);
    *i = jl_unbox_int32(v);
}

void
getConfigArr_str(char *resArr[], const char *name)
{
    //get len
    jl_value_t* v = jl_eval_string(name);
    int len = jl_array_len(v);

    //ensure large enough execStr
    char execStr[sizeof(name)+10];
    jl_value_t* jl_var;
    char d[2];
    for (int i = 0; i < len; i++) {
        //add one to d since julia counts from 1
        sprintf(d, "%d", i+1);
        //clear execStr
        memset(execStr, 0, strlen(execStr));
        //add prefix
        strcpy(execStr, name);
        strcat(execStr, "[");
        strcat(execStr, d);
        strcat(execStr, "]");

        //call julia
        jl_var = jl_eval_string(execStr);
        resArr[i] = (const char *)jl_string_ptr(jl_var);
    }
}

void
getConfigArr_float(float *resArr, const char *name)
{
    //get len
    jl_value_t* v = jl_eval_string(name);
    int len = jl_array_len(v);

    //ensure large enough execStr
    char execStr[sizeof(name)+10];
    jl_value_t* jl_var;
    char d[2];
    for (int i = 0; i < len; i++) {
        //add one to d since julia counts from 1
        sprintf(d, "%d", i+1);
        //clear execStr
        memset(execStr, 0, strlen(execStr));
        //add prefix
        strcpy(execStr, name);
        strcat(execStr, "[");
        strcat(execStr, d);
        strcat(execStr, "]");

        //call julia
        jl_var = jl_eval_string(execStr);
        resArr[i] = (const float*)jl_string_ptr(jl_var);
    }
}

void
getConfigArr_int(int *resArr, const char *name)
{
    //get len
    jl_value_t* v = jl_eval_string(name);
    int len = jl_array_len(v);

    //ensure large enough execStr
    char execStr[sizeof(name)+10];
    jl_value_t* jl_var;
    char d[2];
    for (int i = 0; i < len; i++) {
        //add one to d since julia counts from 1
        sprintf(d, "%d", i+1);
        //clear execStr
        memset(execStr, 0, strlen(execStr));
        //add prefix
        strcpy(execStr, name);
        strcat(execStr, "[");
        strcat(execStr, d);
        strcat(execStr, "]");

        //call julia
        jl_var = jl_eval_string(execStr);
        resArr[i] = (int *)jl_string_ptr(jl_var);
    }
}

void
getConfigHotkeys(Hotkey *hotkeys, const char *name)
{
    int len = getLen(name);
    char execStr[10];
    jl_value_t *t;
    char d[2];
    for (int i = 0; i < len; i++) {
        // setup strings
        sprintf(d, "%d", i + 1);
        memset(execStr, 0, strlen(execStr));
        strcpy(execStr, "arr[");
        strcat(execStr, d);
        strcat(execStr, "]");
        char execStrFunc[10];
        strcpy(execStrFunc, execStr);
        strcat(execStr, "[1]");
        strcat(execStrFunc, "[2]");

        //save value and function pointer
        t = jl_eval_string(execStr);
        hotkeys[i].symbol = (const char *)jl_string_ptr(t);
        hotkeys[i].func = jl_get_function(jl_base_module, execStrFunc);
    }
}

void
getConfigLayouts(Layout *layouts, const char *name)
{
    int len = getLen(name);
    char execStr[10];
    jl_value_t *t;
    char d[2];
    for (int i = 0; i < len; i++) {
        // setup strings
        sprintf(d, "%d", i + 1);
        memset(execStr, 0, strlen(execStr));
        strcpy(execStr, "arr[");
        strcat(execStr, d);
        strcat(execStr, "]");
        char execStrFunc[10];
        strcpy(execStrFunc, execStr);
        strcat(execStr, "[1]");
        strcat(execStrFunc, "[2]");

        //save value and function pointer
        layouts[i].symbol = getConfigArr_str(&layouts[i].symbol, execStr);
        layouts[i].arrange = jl_get_function(jl_base_module, execStrFunc);
    }
}

void getConfigMonRules(MonitorRule *monrules, const char *name)
{
    int len = getLen(name);
    char execStr[10];
    jl_value_t *t;
    char d[2];
    for (int i = 0; i < len; i++) {
        // setup strings
        sprintf(d, "%d", i + 1);
        memset(execStr, 0, strlen(execStr));
        strcpy(execStr, "arr[");
        strcat(execStr, d);
        strcat(execStr, "]");
        char execStrFunc[10];
        strcpy(execStrFunc, execStr);
        strcat(execStr, "[1]");
        strcat(execStrFunc, "[2]");

        //save value and function pointer
        t = jl_eval_string(execStr);
        monrules[i].lt = (const char *)jl_string_ptr(t);
        monrules[i]. = jl_get_function(jl_base_module, execStrFunc);
    }
}
