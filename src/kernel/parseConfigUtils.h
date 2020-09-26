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
char** getConfigStrArr(char *name);
float* getConfigFloatArr(char *name);
int* getConfigIntArr(char *name);

//special
Hotkey getConfigHotkeys(char *name);
Rule getConfigRules(char *name);
Layout getConfigLayouts(char *name);
MonitorRule getConfigMonRules(char *name);

//utils
char* arrayPosToStr(char *resStr, char *varName, int i);
char* array2DPosToStr(char *resStr, char *varName, int i, int j);
#endif /* PARSE_CONFIG_UTILS_H */
