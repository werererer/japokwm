#include "utils/parseConfigUtils.h"
#include <julia.h>
#include <stdlib.h>
#include <string.h>

static int jlArrLen(char *module, char *name)
{
    jl_array_t *arr = (jl_array_t*)evalString(module, name);
    return jl_array_len(arr);
}

jl_value_t *evalString(char *module, const char *name) {
    char res[NUM_CHARS];
    if (strcmp(module, "") != 0) {
        strcpy(res, module);
        strcat(res, ".");
        strcat(res, name);
    }
    return jl_eval_string(res);
}

void initConfig(char *module, char *path)
{
    char *prefix = "include(\"";
    char *postfix = "\")";
    //pluse one for the null char after string
    // int charPtrlen = strlen(path) + strlen(prefix) + strlen(postfix) + 1;
    char res[30];
    strcpy(res, prefix);
    strcat(res, path);
    strcat(res, postfix);
    evalString(module, res);
}

char *getConfigStr(char *module, char *name)
{
    jl_value_t *v = evalString(module, name);
    return (char *)jl_string_ptr(v);
}

float getConfigFloat(char *module, char *name)
{
    jl_value_t *v = evalString(module, name);
    return jl_unbox_float64(v);
}

int getConfigInt(char *module, char *name)
{
    jl_value_t* v = evalString(module, name);
    return jl_unbox_int32(v);
}

jl_function_t *getConfigFunc(char *module, char *name)
{
    jl_function_t *v = evalString(module, name);
    return v;
}

struct layout getConfigLayout(char *module, char *name)
{
    struct layout layout;
    int len = ARR_STRING_LENGTH(name);
    char execStr1[len];
    char execStr2[len];

    arrayPosToStr(execStr1, name, 1);
    arrayPosToStr(execStr2, name, 2);

    layout.symbol = getConfigStr(module, execStr1);
    printf("symbol %s\n", layout.symbol);
    layout.arrange = getConfigFunc(module, execStr2);
    return layout;
}

struct rule getConfigRule(char *module, char *name)
{
    struct rule rule;
    int len = ARR_STRING_LENGTH(name);
    char execStr1[len];
    char execStr2[len];
    char execStr3[len];
    char execStr4[len];
    char execStr5[len];

    arrayPosToStr(execStr1, name, 1);
    arrayPosToStr(execStr2, name, 2);
    arrayPosToStr(execStr3, name, 3);
    arrayPosToStr(execStr4, name, 4);
    arrayPosToStr(execStr5, name, 5);

    rule.id = getConfigStr(module, execStr1);
    rule.floating = getConfigInt(module, execStr2);
    rule.monitor = getConfigInt(module, execStr3);
    rule.tags = getConfigInt(module, execStr4);
    rule.title = getConfigStr(module, execStr5);
    return rule;
}

struct monRule getConfigMonRule(char *module, char *name)
{
    struct monRule monrule;

    int len = ARR_STRING_LENGTH(name);
    char execStr1[len];
    char execStr2[len];
    char execStr3[len];
    char execStr4[len];
    char execStr5[len];
    char execStr6[len];

    arrayPosToStr(execStr1, name, 1);
    arrayPosToStr(execStr2, name, 2);
    arrayPosToStr(execStr3, name, 3);
    arrayPosToStr(execStr4, name, 4);
    arrayPosToStr(execStr5, name, 5);
    arrayPosToStr(execStr6, name, 6);

    monrule.name = getConfigStr(module, execStr1);
    monrule.mfact = getConfigFloat(module, execStr2);
    monrule.nmaster = getConfigInt(module, execStr3);
    monrule.scale = getConfigFloat(module, execStr4);
    getConfigLayoutArr(module, monrule.lt, execStr5);
    monrule.rr = getConfigInt(module, execStr6);
    return monrule;
}

Key getConfigKey(char *module, char *name)
{
    Key key;
    int len = ARR_STRING_LENGTH(name);
    char execStr1[len];
    char execStr2[len];

    arrayPosToStr(execStr1, name, 1);
    arrayPosToStr(execStr2, name, 2);

    key.symbol = getConfigStr(module, execStr1);
    key.func = getConfigFunc(module, execStr2);
    return key;
}

void getConfigStrArr(char *module, char **resArr, char *name)
{
    char execStr[ARR_STRING_LENGTH(name)];
    int len = jlArrLen(module, name);

    for (int i = 0; i < len; i++) {
        arrayPosToStr(execStr, name, i+1);
        resArr[i] = getConfigStr(module, execStr);
    }
}

void getConfigIntArr(char *module, int resArr[], char *name)
{
    int len = jlArrLen(module, name);
    char execStr[ARR_STRING_LENGTH(name)];

    for (int i = 0; i < len; i++) {
        arrayPosToStr(execStr, name, i+1);
        resArr[i] = getConfigInt(module, execStr);
    }
}

void getConfigFloatArr(char *module, float resArr[], char *name)
{
    int len = jlArrLen(module, name);
    char execStr[ARR_STRING_LENGTH(name)];

    for (int i = 0; i < len; i++) {
        arrayPosToStr(execStr, name, i+1);
        resArr[i] = getConfigFloat(module, execStr);
    }
}

void getConfigKeyArr(char *module, Key *keys, char *name)
{
    int len = jlArrLen(module, name);
    char execStr[ARR_STRING_LENGTH(name)];

    for (int i = 0; i < len; i++) {
        arrayPosToStr(execStr, name, i+1);
        keys[i] = getConfigKey(module, execStr);
    }
}

void getConfigRuleArr(char *module, struct rule *rules, char *name)
{
    int len = jlArrLen(module, name);
    char execStr[ARR_STRING_LENGTH(name)];

    for (int i = 0; i < len; i++) {
        arrayPosToStr(execStr, name, i+1);
        rules[i] = getConfigRule(module, execStr);
    }
}

void getConfigLayoutArr(char *module, struct layout *layouts, char *name)
{
    int len = jlArrLen(module, name);
    char execStr[ARR_STRING_LENGTH(name)];

    for (int i = 1; i <= len; i++) {
        arrayPosToStr(execStr, name, i);
        layouts[i-1] = getConfigLayout(module, execStr);
    }
}

void getConfigMonRuleArr(char *module, struct monRule *monrules, char *name)
{
    int len = jlArrLen(module, name);
    char execStr[ARR_STRING_LENGTH(name)];
    for (int i = 0; i < len; i++) {
        arrayPosToStr(execStr, name, i+1);
        monrules[i] = getConfigMonRule(module, execStr);
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
