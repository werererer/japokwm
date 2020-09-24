#include "parseconfig.h"

int getLen(const char *name) {
    jl_value_t *arr = jl_eval_string("arr");
    return jl_array_len(arr);
}

void initConfig(const char *path)
{
    jl_eval_string(strcat(strcat("include\"", path),"\")"));
}

// get variable
void getConfig_str(char *value, const char *name)
{
    jl_value_t* v = jl_eval_string(name);
    strcpy(value, jl_string_ptr(v));
}

void getConfig_float(float *f, const char *name)
{
    jl_value_t* v = jl_eval_string(name);
    *f = jl_unbox_float32(v);
}

void getConfig_int(int *i, const char *name)
{
    jl_value_t* v = jl_eval_string(name);
    *i = jl_unbox_int32(v);
}

void getConfig_func(jl_function_t **func, const char *name)
{
    *func = jl_get_function(jl_base_module, name);
}

void getConfigArr_str(char *resArr[], const char *name)
{
    //get len
    jl_value_t* v = jl_eval_string(name);
    int len = jl_array_len(v);

    //ensure large enough execStr
    char execStr[sizeof(name)+10];
    jl_value_t* jl_var;
    char d[5];
    for (int i = 0; i < len; i++) {
        arrayPosToStr(execStr, name, i);
        getConfigArr_str(&resArr[i], execStr);
    }
}

void getConfigArr_float(float resArr[], const char *name)
{
    //get len
    jl_value_t* v = jl_eval_string(name);
    int len = jl_array_len(v);

    //ensure large enough execStr
    char execStr[sizeof(name)+10];
    jl_value_t* jl_var;
    char d[2];
    for (int i = 0; i < len; i++) {
        arrayPosToStr(execStr, name, i);
        getConfig_float(&resArr[i], execStr);
    }
}

void getConfigArr_int(int *resArr, const char *name)
{
    //get len
    jl_value_t* v = jl_eval_string(name);
    int len = jl_array_len(v);

    //ensure large enough execStr
    char execStr[sizeof(name)+10];
    jl_value_t* jl_var;
    char d[2];
    for (int i = 0; i < len; i++) {
        arrayPosToStr(execStr, name, i);
        getConfig_int(&resArr[i], execStr);
    }
}

void getConfigHotkeys(Hotkey *hotkeys, const char *name)
{
    int len = getLen(name);
    char execStr[10];
    char execStrFunc[10];
    jl_value_t *t;
    char d[2];

    for (int i = 0; i < len; i++) {
        array2DPosToStr(execStr, name, i, 1);
        array2DPosToStr(execStrFunc, name, i, 2);

        getConfig_str((char*)hotkeys[i].symbol, execStr);
        getConfig_func(&hotkeys[i].func, execStrFunc);
    }
}

void getConfigLayouts(Layout *layouts, const char *name)
{
    int len = getLen(name);
    char execStr[10];
    char execStrFunc[10];
    jl_value_t *t;
    char d[2];
    for (int i = 0; i < len; i++) {
        array2DPosToStr(execStr, name, i, 1);
        array2DPosToStr(execStrFunc, name, i, 2);

        getConfig_str(layouts[i].symbol, execStr);
        getConfig_func(&layouts[i].arrange, execStrFunc);
    }
}
void getConfigMonRules(MonitorRule *monrules, const char *name)
{
    int len = getLen(name);
    char execStr0[10];
    char execStr1[10];
    char execStr2[10];
    char execStr3[10];
    char execStr4[10];
    char execStr5[10];
    jl_value_t *t;
    char d[2];
    for (int i = 0; i < len; i++) {
        array2DPosToStr(execStr0, name, i, 0);
        array2DPosToStr(execStr1, name, i, 1);
        array2DPosToStr(execStr2, name, i, 2);
        array2DPosToStr(execStr3, name, i, 3);
        array2DPosToStr(execStr4, name, i, 4);
        array2DPosToStr(execStr5, name, i, 5);

        getConfig_str((char*)monrules->name, execStr0);
        getConfig_float(&monrules->mfact, execStr1);
        getConfigArr_int(&monrules->nmaster, execStr2);
        getConfigArr_float(&monrules->scale, execStr3);
        getConfigLayouts(monrules->lt, execStr4);
        getConfigArr_int((int*)&monrules->rr, execStr5);
    }
}

//utils
/*
 * convert an integer to a char* with brackets.
 * Example 1 -> "[1]"
 * */
static void appendIndex(char *resStr, int i)
{
    char d[5];
    //add one since julia's arrays are 1 based
    sprintf(d, "%d", i+1);
    strcat(resStr, "[");
    strcat(resStr, d);
    strcat(resStr, "]");
}

/* 
 * convert two integers to a char* with brackets.
 * Example 1 2 -> "[1][2]"
 * */
static void append2DIndex(char *resStr, int i, int j)
{
    appendIndex(resStr, i);
    appendIndex(resStr, j);
}

static void clearStr(char *resStr)
{
    memset(resStr, 0, strlen(resStr));
}

void arrayPosToStr(char *resStr, const char *varName, int i)
{
    clearStr(resStr);
    strcpy(resStr, varName);
    appendIndex(resStr, i);
}

void array2DPosToStr(char *resStr, const char *varName, int i, int j)
{
    clearStr(resStr);
    strcpy(resStr, varName);
    append2DIndex(resStr, i, j);
}
