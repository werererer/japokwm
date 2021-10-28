#include <stdio.h>
#include <glib.h>

#include "bitset/bitset.h"

void test_bitset()
{
    BitSet *bitset = bitset_create();
    g_assert_cmpint(bitset_test(bitset, 0), ==, false);
    g_assert_cmpint(bitset_test(bitset, 20), ==, false);
    bitset_set(bitset, 1);
    g_assert_cmpint(bitset_test(bitset, 1), ==, true);
    bitset_set(bitset, 20);
    g_assert_cmpint(bitset_test(bitset, 20), ==, true);
}

void test_bitset_reverse()
{
    BitSet *bitset = bitset_create();
    bitset_assign(bitset, 0, true);
    bitset_assign(bitset, 1, false);
    bitset_assign(bitset, 2, true);
    bitset_assign(bitset, 3, true);
    bitset_assign(bitset, 4, false);
    bitset_reverse(bitset, 0, 5);
}

void test_bitset_from_value_reversed()
{
    // 123 == 0b01111011
    // reversed(123) == 0b11011110
    int test_value = 123;
    BitSet *bitset_normal = bitset_from_value(test_value);
    BitSet *bitset_reversed = bitset_from_value_reversed(test_value);

    g_assert_cmpint(bitset_test(bitset_normal, 0), ==, 1);
    g_assert_cmpint(bitset_test(bitset_normal, 1), ==, 1);
    g_assert_cmpint(bitset_test(bitset_normal, 2), ==, 0);
    g_assert_cmpint(bitset_test(bitset_normal, 3), ==, 1);
    g_assert_cmpint(bitset_test(bitset_normal, 4), ==, 1);
    g_assert_cmpint(bitset_test(bitset_normal, 5), ==, 1);
    g_assert_cmpint(bitset_test(bitset_normal, 6), ==, 1);
    g_assert_cmpint(bitset_test(bitset_normal, 7), ==, 0);

    g_assert_cmpint(bitset_test(bitset_reversed, 0), ==, 0);
    g_assert_cmpint(bitset_test(bitset_reversed, 1), ==, 1);
    g_assert_cmpint(bitset_test(bitset_reversed, 2), ==, 1);
    g_assert_cmpint(bitset_test(bitset_reversed, 3), ==, 1);
    g_assert_cmpint(bitset_test(bitset_reversed, 4), ==, 1);
    g_assert_cmpint(bitset_test(bitset_reversed, 5), ==, 0);
    g_assert_cmpint(bitset_test(bitset_reversed, 6), ==, 1);
    g_assert_cmpint(bitset_test(bitset_reversed, 7), ==, 1);

    bitset_reverse(bitset_reversed, 0, 64);
    g_assert_cmpint(bitset_test(bitset_reversed, 0), ==, 1);
    g_assert_cmpint(bitset_test(bitset_reversed, 1), ==, 1);
    g_assert_cmpint(bitset_test(bitset_reversed, 2), ==, 0);
    g_assert_cmpint(bitset_test(bitset_reversed, 3), ==, 1);
    g_assert_cmpint(bitset_test(bitset_reversed, 4), ==, 1);
    g_assert_cmpint(bitset_test(bitset_reversed, 5), ==, 1);
    g_assert_cmpint(bitset_test(bitset_reversed, 6), ==, 1);
    g_assert_cmpint(bitset_test(bitset_reversed, 7), ==, 0);
}

void test_bitset_xor()
{
    BitSet *bitset1 = bitset_create();
    BitSet *bitset2 = bitset_create();

    bitset_set(bitset1, 1);
    bitset_set(bitset2, 1);

    bitset_xor(bitset1, bitset2);
    g_assert_cmpint(bitset_test(bitset1, 0), ==, false);
    g_assert_cmpint(bitset_test(bitset1, 1), ==, false);
    g_assert_cmpint(bitset_test(bitset1, 2), ==, false);

    bitset_reset_all(bitset1);
    bitset_reset_all(bitset2);

    bitset_set(bitset2, 1);

    bitset_xor(bitset1, bitset2);
    g_assert_cmpint(bitset_test(bitset1, 1), ==, true);
}

void test_bitset_and()
{
    BitSet *bitset1 = bitset_create();
    BitSet *bitset2 = bitset_create();
    bitset_set(bitset1, 1);
    bitset_set(bitset2, 1);

    bitset_and(bitset1, bitset2);

    g_assert_cmpint(bitset_any(bitset1), ==, true);

    bitset_reset_all(bitset1);
    bitset_reset_all(bitset2);

    bitset_set(bitset1, 1);
    bitset_set(bitset2, 2);

    bitset_and(bitset1, bitset2);
    g_assert_cmpint(bitset_any(bitset1), ==, false);
}

void test_copy_bitset()
{
    BitSet *bitset1 = bitset_create();
    BitSet *bitset2 = bitset_copy(bitset1);
    int bit = bitset_test(bitset2, 0);
    g_assert_cmpint(bit, ==, 0);
}

void test_bitset_assign()
{
    BitSet *bitset1 = bitset_create();
    bitset_assign(bitset1, 0, 1);
    g_assert_cmpint(bitset_test(bitset1, 0), ==, 1);
}

void test_from_value()
{
    /* // 42 == 0b101010 */
    /* BitSet *bitset1 = bitset_from_value_reversed(42, 6); */
    /* g_assert_cmpint(bitset_test(bitset1, 0), ==, 1); */
    /* g_assert_cmpint(bitset_test(bitset1, 1), ==, 0); */
    /* g_assert_cmpint(bitset_test(bitset1, 2), ==, 1); */
    /* g_assert_cmpint(bitset_test(bitset1, 3), ==, 0); */
    /* g_assert_cmpint(bitset_test(bitset1, 4), ==, 1); */
    /* g_assert_cmpint(bitset_test(bitset1, 5), ==, 0); */
}

#define PREFIX "bitset"
#define add_test(func) g_test_add_func("/"PREFIX"/"#func, func)
int main(int argc, char** argv)
{
    setbuf(stdout, NULL);
    g_test_init(&argc, &argv, NULL);

    add_test(test_bitset);
    add_test(test_bitset_xor);
    add_test(test_bitset_and);
    add_test(test_copy_bitset);
    add_test(test_bitset_assign);
    /* add_test(test_from_value); */

    return g_test_run();
}
