#include <stdlib.h>
#include <check.h>
#include <string.h>

#include "utils/stringUtils.h"

START_TEST(intToStringTest)
{
    char res[NUM_DIGITS];
    int_to_string(res, 30);
    ck_assert_str_eq("30", res);
} END_TEST

START_TEST(doubleToStringTest)
{
    char res[MAXLEN];
    double_to_string(res, 3.141592);
    ck_assert_str_eq("3.142", res);
} END_TEST

START_TEST(repeatStringTest)
{
    char res[NUM_CHARS];
    strcpy(res, "g");
    repeat_string(res, 4);
    ck_assert_str_eq("gggg", res);
} END_TEST

Suite *suite()
{
    Suite *s;
    TCase *tc;

    s = suite_create("stringUtils");
    tc = tcase_create("Core");

    tcase_add_test(tc, doubleToStringTest);
    tcase_add_test(tc, intToStringTest);
    tcase_add_test(tc, repeatStringTest);
    suite_add_tcase(s, tc);

    return s;
}

int main()
{
    int numberFailed;
    Suite *s;
    SRunner *sr;

    s = suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    srunner_ntests_run(sr);
    numberFailed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (numberFailed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
