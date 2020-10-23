#include "utils/parseConfigUtils.h"
#include <julia.h>
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
    // int charPtrlen = strlen(path) + strlen(prefix) + strlen(postfix) + 1;
    char res[30];
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
    return jl_unbox_float64(v);
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

Layout getConfigLayout(char *name)
{
    Layout layout;
    int len = ARR_STRING_LENGTH(name);
    char execStr1[len];
    char execStr2[len];

    arrayPosToStr(execStr1, name, 1);
    arrayPosToStr(execStr2, name, 2);

    layout.symbol = getConfigStr(execStr1);
    layout.arrange = getConfigFunc(execStr2);
    return layout;
}

void setConfigLayoutSymbol(char *name, Layout l)
{
    // add 4 for chars: ' = ' and \0
    int len = strlen(name) + strlen(l.symbol) + 4;
    char execStr[len];
    strcpy(execStr, name);
    strcat(execStr, " = ");
    strcat(execStr, l.symbol);
    jl_eval_string(execStr);
}

static jl_value_t* getWlrBox(struct wlr_box w)
{

    jl_datatype_t *wlrBox = (jl_datatype_t *)jl_eval_string("Layouts.wlr_fbox");
    jl_value_t *sX = jl_box_int32(w.x);
    jl_value_t *sY = jl_box_int32(8);
    jl_value_t *sWidth = jl_box_int32(w.width);
    jl_value_t *sHeight = jl_box_int32(w.height);
    jl_value_t *str = jl_new_struct(wlrBox, sX, sY, sWidth, sHeight);
    return str;
}

struct rule getConfigRule(char *name)
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

    rule.id = getConfigStr(execStr1);
    rule.isfloating = getConfigInt(execStr2);
    rule.monitor = getConfigInt(execStr3);
    rule.tags = getConfigInt(execStr4);
    rule.title = getConfigStr(execStr5);
    return rule;
}

struct monRule getConfigMonRule(char *name)
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

    monrule.name = getConfigStr(execStr1);
    monrule.mfact = getConfigFloat(execStr2);
    monrule.nmaster = getConfigInt(execStr3);
    monrule.scale = getConfigFloat(execStr4);
    getConfigLayoutArr(monrule.lt, execStr5);
    monrule.rr = getConfigInt(execStr6);
    return monrule;
}

Key getConfigKey(char *name)
{
    Key key;
    int len = ARR_STRING_LENGTH(name);
    char execStr1[len];
    char execStr2[len];

    arrayPosToStr(execStr1, name, 1);
    arrayPosToStr(execStr2, name, 2);

    key.symbol = getConfigStr(execStr1);
    key.func = getConfigFunc(execStr2);
    return key;
}

jl_value_t* toJlMonitor(char *name, struct monitor *m)
{
    jl_datatype_t *t = (jl_datatype_t *)jl_eval_string(name);
    jl_value_t *sM = getWlrBox(m->m);
    jl_value_t *sW = getWlrBox(m->w);
    jl_value_t *sTagset = jl_box_uint64(m->seltags);
    jl_value_t *sMfact = jl_box_float64(m->mfact);
    jl_value_t *sNmaster = jl_box_uint64(m->nmaster);
    jl_value_t *res = jl_new_struct(t, sM, sW, sTagset, sMfact, sNmaster);
    return res;
}

void getConfigStrArr(char **resArr, char *name)
{
    char execStr[ARR_STRING_LENGTH(name)];
    int len = jlArrLen(name);

    for (int i = 0; i < len; i++) {
        arrayPosToStr(execStr, name, i+1);
        resArr[i] = getConfigStr(execStr);
    }
}

void getConfigIntArr(int resArr[], char *name)
{
    int len = jlArrLen(name);
    char execStr[ARR_STRING_LENGTH(name)];

    for (int i = 0; i < len; i++) {
        arrayPosToStr(execStr, name, i+1);
        resArr[i] = getConfigInt(execStr);
    }
}

void getConfigFloatArr(float resArr[], char *name)
{
    int len = jlArrLen(name);
    char execStr[ARR_STRING_LENGTH(name)];

    for (int i = 0; i < len; i++) {
        arrayPosToStr(execStr, name, i+1);
        resArr[i] = getConfigFloat(execStr);
    }
}

void getConfigKeyArr(Key *keys, char *name)
{
    int len = jlArrLen(name);
    char execStr[ARR_STRING_LENGTH(name)];

    for (int i = 0; i < len; i++) {
        arrayPosToStr(execStr, name, i+1);
        keys[i] = getConfigKey(execStr);
    }
}

void getConfigRuleArr(struct rule *rules, char *name)
{
    int len = jlArrLen(name);
    char execStr[ARR_STRING_LENGTH(name)];

    for (int i = 0; i < len; i++) {
        arrayPosToStr(execStr, name, i+1);
        rules[i] = getConfigRule(execStr);
    }
}

void getConfigLayoutArr(Layout *layouts, char *name)
{
    int len = jlArrLen(name);
    char execStr[ARR_STRING_LENGTH(name)];

    for (int i = 1; i <= len; i++) {
        arrayPosToStr(execStr, name, i);
        layouts[i-1] = getConfigLayout(execStr);
    }
}

void getConfigMonRuleArr(struct monRule *monrules, char *name)
{
    int len = jlArrLen(name);
    char execStr[ARR_STRING_LENGTH(name)];
    for (int i = 0; i < len; i++) {
        arrayPosToStr(execStr, name, i+1);
        monrules[i] = getConfigMonRule(execStr);
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
