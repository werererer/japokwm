#ifndef PARSECONFIG
#define PARSECONFIG
#include "coreUtils.c"

void initConfig(const char *path);
//basic
void getConfig_str(char *value, const char *name);
void getConfig_float(float *f, const char *name);
void getConfig_int(int *i, const char *name);

//array
void getConfigArr_str(char *resArr[], const char *name);
void getConfigArr_float(float *resArr, const char *name);
void getConfigArr_int(int *resArr, const char *name);

//special
void getConfigHotKeys(Hotkey *hotkeys, const char *name);
void getConfigRules(Rule *rules, const char *name);
void getConfigLayouts(Layout *layouts, const char *name);
void getConfigMonRules(MonitorRule *monrules, const char *name);
#endif
