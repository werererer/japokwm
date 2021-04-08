#include "utils/stringUtils.h"
#include <stdio.h>
#include <string.h>
#include <wlr/util/log.h>

void int_to_string(char *res, int i)
{
    sprintf(res, "%d", i);
}

void double_to_string(char *res, double f)
{
    sprintf(res, "%.3f", f);
}

void repeat_string(char *str, int n)
{
    char res[NUM_CHARS] = "";
    for (int i = 0; i < n; i++) {
        strcat(res, str);
    }
    strcpy(str, res);
}
