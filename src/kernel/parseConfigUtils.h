#ifndef PARSE_CONFIG_UTILS_H
#define PARSE_CONFIG_UTILS_H
#include "coreUtils.h"
#include <string.h>

#define NUM_DIGITS 9
#define ARR1D_STRING_LENGTH(X) strlen(X) + strlen("[]") + NUM_DIGITS
#define ARR2D_STRING_LENGTH(X) strlen(X) + 2*(strlen("[]") + NUM_DIGITS)
//TODO: define foreach julia

void initConfig(char *path);
//basic
char* getConfigStr(char *name);
float getConfigFloat(char *name);
int getConfigInt(char *name);

//array
void getConfigStrArr(char **resArr, char *name);
void getConfigFloatArr(float *resArr, char *name);
void getConfigIntArr(int *resArr, char *name);

//special
void getConfigHotkeys(Hotkey *hotkeys, char *name);
void getConfigRules(Rule *rules, char *name);
void getConfigLayouts(Layout *layouts, char *name);
void getConfigMonRules(MonitorRule *monrules, char *name);

//utils
void arrayPosToStr(char *resStr, char *varName, int i);
void array2DPosToStr(char *resStr, char *varName, int i, int j);
#endif /* PARSE_CONFIG_UTILS_H */
