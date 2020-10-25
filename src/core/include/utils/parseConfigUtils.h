#ifndef PARSE_CONFIG_UTILS_H
#define PARSE_CONFIG_UTILS_H
#include "monitor.h"
#include <string.h>
#include "utils/coreUtils.h"

void initConfig(char *path);
// basic
// get
Key getConfigKey(char *name);
struct layout getConfigLayout(char *name);
struct monRule getConfigMonRule(char *name);
struct rule getConfigRule(char *name);
char* getConfigStr(char *name);
float getConfigFloat(char *name);
int getConfigInt(char *name);
jl_function_t* getConfigFunc(char *name);
jl_value_t* toJlMonitor(char *name, struct monitor *m);
// set
void setConfigLayoutSymbol(char *name, struct layout l);

// array
void getConfigStrArr(char **resArr, char *name);
void getConfigFloatArr(float *resArr, char *name);
void getConfigIntArr(int *resArr, char *name);
void getConfigLayoutArr(struct layout *layouts, char *name);
void getConfigKeyArr(Key *keys, char *name);
void getConfigRuleArr(struct rule *rules, char *name);
void getConfigMonRuleArr(struct monRule *monrules, char *name);

//utils
void arrayPosToStr(char *resStr, char *varName, int i);
void array2DPosToStr(char *resStr, char *varName, int i, int j);

#endif /* PARSE_CONFIG_UTILS_H */
