#include <stdlib.h>
#include <glib.h>
#include <string.h>

#include "utils/stringUtils.h"

void intToStringTest()
{
    char res[NUM_DIGITS];
    int_to_string(res, 30);
    g_assert_cmpstr("30", ==, res);
}

void doubleToStringTest()
{
    char res[MAXLEN];
    double_to_string(res, 3.141592);
    g_assert_cmpstr("3.142", ==, res);
}

void repeatStringTest()
{
    char res[NUM_CHARS];
    strcpy(res, "g");
    repeat_string(res, 4);
    g_assert_cmpstr("gggg", ==, res);
}

#define PREFIX "stringUtils"
#define add_test(func) g_test_add_func("/"PREFIX"/"#func, func)
int main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    g_test_init(&argc, &argv, NULL);

    add_test(doubleToStringTest);
    add_test(intToStringTest);
    add_test(repeatStringTest);

    return g_test_run();
}
