#ifndef PARSE_CONFIG_UTILS_H
#define PARSE_CONFIG_UTILS_H
#include <string.h>
#include "utils/coreUtils.h"
#include "monitor.h"

void initConfig(char *module, char *path);
jl_value_t *evalString(char *module, const char *name);
Key getConfigKey(char *module, char *name);
struct layout getConfigLayout(char *module, char *name);
struct monRule getConfigMonRule(char *module, char *name);
struct rule getConfigRule(char *module, char *name);
char* getConfigStr(char *module, char *name);
float getConfigFloat(char *module, char *name);
int getConfigInt(char *module, char *name);
jl_function_t* getConfigFunc(char *module, char *name);

// array
void getConfigStrArr(char *module, char **resArr, char *name);
void getConfigFloatArr(char *module, float *resArr, char *name);
void getConfigIntArr(char *module, int *resArr, char *name);
void getConfigLayoutArr(char *module, struct layout *layouts, char *name);
void getConfigKeyArr(char *module, Key *keys, char *name);
void getConfigRuleArr(char *module, struct rule *rules, char *name);
void getConfigMonRuleArr(char *module, struct monRule *monrules, char *name);

//utils
void arrayPosToStr(char *resStr, char *varName, int i);
void array2DPosToStr(char *resStr, char *varName, int i, int j);

#endif /* PARSE_CONFIG_UTILS_H */
