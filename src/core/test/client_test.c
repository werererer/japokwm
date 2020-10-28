#include <check.h>

#include "client.h"

START_TEST(testVisibleon)
{
    struct client c;
    struct monitor m;

    tagsetCreate(&c.tagset);
    tagsetCreate(&m.tagset);

    c.mon = &m;
    c.tagset.selTags[0] = 7;
    m.tagset.selTags[0] = 8;
    ck_assert_int_eq(visibleon(&c, &m), false);
    c.tagset.selTags[0] = 5;
    m.tagset.selTags[0] = 3;
    m.tagset.focusedTag = 1;
    ck_assert_int_eq(visibleon(&c, &m), true);

    tagsetDestroy(&m.tagset);
    tagsetDestroy(&c.tagset);
} END_TEST

Suite *suite() {
    Suite *s;
    TCase *tc;

    s = suite_create("client");
    tc = tcase_create("core");

    tcase_add_test(tc, testVisibleon);
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
