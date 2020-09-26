#include "parseConfigUtils.h"
#include <stdlib.h>
#include <string.h>

static int jlArrLen(char *name) {
  jl_value_t *arr = jl_eval_string(name);
  return jl_array_len(arr);
}

void initConfig(char *path)
{
    char *prefix = "include(\"";
    char *postfix = "\")";
    //pluse one for the null char after string
    int charPtrlen = strlen(path) + strlen(prefix) + strlen(postfix) + 1;
    char res[charPtrlen];
    strcpy(res, prefix);
    strcat(res, path);
    strcat(res, postfix);
    jl_eval_string(res);
}

char* getConfigStr(char *name)
{
    jl_value_t* v = jl_eval_string(name);
    return (char *)jl_string_ptr(v);
}

float getConfigFloat(char *name)
{
    jl_value_t* v = jl_eval_string(name);
    return jl_unbox_float32(v);
}

int getConfigInt(char *name)
{
    jl_value_t* v = jl_eval_string(name);
    return jl_unbox_int32(v);
}

jl_function_t* getConfigFunc(char *name)
{
    return jl_eval_string(name);
}

void getConfigStrArr(char **resArr, char *name)
{
    char execStr[ARR1D_STRING_LENGTH(name)];
    int len = jlArrLen(name);

    for (int i = 1; i <= len; i++) {
        arrayPosToStr(execStr, name, i);
        resArr[i-1] = getConfigStr(execStr);
    }
}

void getConfigIntArr(int resArr[], char *name)
{
    int len = jlArrLen(name);
    char execStr[ARR1D_STRING_LENGTH(name)];

    for (int i = 1; i <= len; i++) {
        arrayPosToStr(execStr, name, i);
        resArr[i-1] = getConfigInt(execStr);
    }
}

void getConfigFloatArr(float resArr[], char *name)
{
    int len = jlArrLen(name);
    char execStr[ARR1D_STRING_LENGTH(name)];

    for (int i = 1; i <= len; i++) {
        arrayPosToStr(execStr, name, i);
        resArr[i] = getConfigFloat(execStr);
    }
}

void getConfigHotkeys(Hotkey *hotkeys, char *name)
{
    int len = jlArrLen(name);
    char execStr[10];
    char execStrFunc[10];
    jl_value_t *t;
    char d[2];

    for (int i = 1; i <= len; i++) {
        array2DPosToStr(execStr, name, i, 1);
        array2DPosToStr(execStrFunc, name, i, 2);

        hotkeys[i-1].symbol = getConfigStr(execStr);
        hotkeys[i-1].func = getConfigFunc(execStrFunc);
    }
}

void getConfigRules(Rule *rules, char *name)
{
    int len = jlArrLen(name);
    char execStr[10];
    // TODO: define rules
}

void getConfigLayouts(Layout *layouts, char *name)
{
    int len = jlArrLen(name);
    char execStr[ARR2D_STRING_LENGTH(name)];
    char execStrFunc[ARR2D_STRING_LENGTH(name)];

    for (int i = 1; i <= len; i++) {
        array2DPosToStr(execStr, name, i, 1);
        array2DPosToStr(execStrFunc, name, i, 2);

        layouts[i-1].symbol = getConfigStr(execStr);
        layouts[i-1].arrange = getConfigFunc(execStrFunc);
    }
}

void getConfigMonRules(MonitorRule *monrules, char *name)
{
    int len = jlArrLen(name);
    char execStr0[ARR2D_STRING_LENGTH(name)];
    char execStr1[ARR2D_STRING_LENGTH(name)];
    char execStr2[ARR2D_STRING_LENGTH(name)];
    char execStr3[ARR2D_STRING_LENGTH(name)];
    char execStr4[ARR2D_STRING_LENGTH(name)];
    char execStr5[ARR2D_STRING_LENGTH(name)];
    jl_value_t *t;
    char d[2];
    for (int i = 1; i <= len; i++) {
        array2DPosToStr(execStr0, name, i, 1);
        array2DPosToStr(execStr1, name, i, 2);
        array2DPosToStr(execStr2, name, i, 3);
        array2DPosToStr(execStr3, name, i, 4);
        array2DPosToStr(execStr4, name, i, 5);
        array2DPosToStr(execStr5, name, i, 6);

        monrules->name = getConfigStr(execStr0);
        monrules->mfact = getConfigFloat(execStr1);
        monrules->nmaster = getConfigInt(execStr2);
        getConfigFloatArr(&monrules->scale, execStr3);
        getConfigLayouts(monrules->lt, execStr4);
        getConfigIntArr((int *)&monrules->rr, execStr5);
    }
}

//utils
/*
 * convert an integer to a char* with brackets.
 * Example 1 -> "[1]"
 * */
void appendIndex(char *resStr, int i)
{
    char d[5];
    sprintf(d, "%d", i);
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

/*
 * convert varname and integer i to string: "varname[i]"
 * */
void arrayPosToStr(char *resStr, char *varName, int i)
{
    strcpy(resStr, (char *)varName);
    appendIndex(resStr, i);
}

/*
 * convert varname, integer i and integer j to string: "varname[i][j]"
 * */
void array2DPosToStr(char *resStr, char *varName, int i, int j)
{
    strcpy(resStr, varName);
    append2DIndex(resStr, i, j);
}
