#include "utils/stringUtils.h"
#include <stdio.h>
#include <string.h>
#include <wlr/util/log.h>

void intToString(char *res, int i)
{
    sprintf(res, "%d", i);
}

void doubleToString(char *res, double f)
{
    sprintf(res, "%.3f", f);
}

void repeatString(char *str, int n)
{
    char res[NUM_CHARS] = "";
    for (int i = 0; i < n; i++) {
        strcat(res, str);
    }
    strcpy(str, res);
}
