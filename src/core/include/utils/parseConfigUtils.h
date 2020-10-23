#ifndef PARSE_CONFIG_UTILS_H
#define PARSE_CONFIG_UTILS_H
#include "utils/coreUtils.h"
#include <string.h>

void initConfig(char *path);
// basic
// get
Key getConfigKey(char *name);
Layout getConfigLayout(char *name);
MonitorRule getConfigMonRule(char *name);
Rule getConfigRule(char *name);
char* getConfigStr(char *name);
float getConfigFloat(char *name);
int getConfigInt(char *name);
jl_function_t* getConfigFunc(char *name);
jl_value_t* toJlMonitor(char *name, struct monitor *m);
// set
void setConfigLayoutSymbol(char *name, Layout l);

// array
void getConfigStrArr(char **resArr, char *name);
void getConfigFloatArr(float *resArr, char *name);
void getConfigIntArr(int *resArr, char *name);
void getConfigLayoutArr(Layout *layouts, char *name);
void getConfigKeyArr(Key *keys, char *name);
void getConfigRuleArr(Rule *rules, char *name);
void getConfigMonRuleArr(MonitorRule *monrules, char *name);

//utils
void arrayPosToStr(char *resStr, char *varName, int i);
void array2DPosToStr(char *resStr, char *varName, int i, int j);


#endif /* PARSE_CONFIG_UTILS_H */
