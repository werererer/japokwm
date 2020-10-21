#include "utils/stringUtils.h"
#include <stdio.h>

void intToString(char *res, int i)
{
    sprintf(res, "%d", i);
}

void doubleToString(char *res, double f)
{
    sprintf(res, "%d", f);
}

void repeatString(char *str, int n)
{
    sprintf(str, "%.*s", n, str);
}
