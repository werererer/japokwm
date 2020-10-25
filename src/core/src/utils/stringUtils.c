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
    sprintf(res, "%f", f);
}

void repeatString(char *str, int n)
{
    char t[64];
    if (sizeof(str) > 64)
        wlr_log(WLR_ERROR, "Too big string in: %s", __func__);
    strcpy(t, str);
    sprintf(str, "%.*s", n, t);
}
