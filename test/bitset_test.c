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

void test_bitset_to_string()
{
    BitSet *bitset = bitset_create();

    g_assert_cmpstr(bitset_to_string(bitset), ==, "00000000");
    bitset_set(bitset, 0);
    g_assert_cmpstr(bitset_to_string(bitset), ==, "00000001");
    bitset_set(bitset, 2);
    g_assert_cmpstr(bitset_to_string(bitset), ==, "00000101");

    bitset_destroy(bitset);
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

void test_bitset_or()
{
    BitSet *bitset1 = bitset_create();
    BitSet *bitset2 = bitset_create();

    bitset_assign(bitset1, 0, false);
    bitset_assign(bitset1, 1, true);
    bitset_assign(bitset1, 2, false);

    bitset_assign(bitset2, 0, false);
    bitset_assign(bitset2, 1, true);
    bitset_assign(bitset2, 2, true);
    bitset_assign(bitset2, 3, true);

    bitset_or(bitset1, bitset2);
    g_assert_cmpint(bitset_test(bitset1, 0), ==, false);
    g_assert_cmpint(bitset_test(bitset1, 1), ==, true);
    g_assert_cmpint(bitset_test(bitset1, 2), ==, true);
    g_assert_cmpint(bitset_test(bitset1, 3), ==, true);
    g_assert_cmpint(bitset_test(bitset1, 4), ==, false);

    bitset_reset_all(bitset1);
    bitset_reset_all(bitset2);

    bitset_assign(bitset1, 0, false);
    bitset_assign(bitset1, 1, true);
    bitset_assign(bitset1, 2, true);
    bitset_assign(bitset1, 3, true);

    bitset_assign(bitset2, 0, false);
    bitset_assign(bitset2, 1, true);
    bitset_assign(bitset2, 2, false);

    bitset_or(bitset1, bitset2);
    g_assert_cmpint(bitset_test(bitset1, 0), ==, false);
    g_assert_cmpint(bitset_test(bitset1, 1), ==, true);
    g_assert_cmpint(bitset_test(bitset1, 2), ==, true);
    g_assert_cmpint(bitset_test(bitset1, 3), ==, true);
    g_assert_cmpint(bitset_test(bitset1, 4), ==, false);
}

void test_bitset_xor()
{
    BitSet *bitset1 = bitset_create();
    BitSet *bitset2 = bitset_create();

    bitset_assign(bitset1, 0, false);
    bitset_assign(bitset1, 1, true);
    bitset_assign(bitset1, 2, false);

    bitset_assign(bitset2, 0, false);
    bitset_assign(bitset2, 1, true);
    bitset_assign(bitset2, 2, true);
    bitset_assign(bitset2, 3, true);

    bitset_xor(bitset1, bitset2);
    g_assert_cmpint(bitset_test(bitset1, 0), ==, false);
    g_assert_cmpint(bitset_test(bitset1, 1), ==, false);
    g_assert_cmpint(bitset_test(bitset1, 2), ==, true);
    g_assert_cmpint(bitset_test(bitset1, 3), ==, true);
    g_assert_cmpint(bitset_test(bitset1, 4), ==, false);

    bitset_reset_all(bitset1);
    bitset_reset_all(bitset2);

    bitset_assign(bitset1, 0, false);
    bitset_assign(bitset1, 1, true);
    bitset_assign(bitset1, 2, true);
    bitset_assign(bitset1, 3, true);

    bitset_assign(bitset2, 0, false);
    bitset_assign(bitset2, 1, true);
    bitset_assign(bitset2, 2, false);

    bitset_xor(bitset1, bitset2);
    g_assert_cmpint(bitset_test(bitset1, 0), ==, false);
    g_assert_cmpint(bitset_test(bitset1, 1), ==, false);
    g_assert_cmpint(bitset_test(bitset1, 2), ==, true);
    g_assert_cmpint(bitset_test(bitset1, 3), ==, true);
    g_assert_cmpint(bitset_test(bitset1, 4), ==, false);
}

void test_bitset_count()
{
    BitSet *bitset = bitset_create();

    // there once was a bug where this resulted into the false count
    bitset_set(bitset, 1);
    bitset_reset(bitset, 1);
    bitset_set(bitset, 1);

    bitset_set(bitset, 10);
    bitset_set(bitset, 10);

    bitset_set(bitset, 12);
    bitset_set(bitset, 15);
    bitset_set(bitset, 3);

    g_assert_cmpint(bitset_count(bitset), ==, 5);
}

void test_bitset_high()
{
    BitSet *bitset = bitset_create();

    bitset_set(bitset, 1);
    bitset_set(bitset, 2);
    bitset_reset(bitset, 2);

    g_assert_cmpint(bitset->high, ==, 1);

    bitset_reset_all(bitset);
    g_assert_cmpint(bitset->high, ==, bitset->low);
}

void test_bitset_low()
{
    BitSet *bitset = bitset_create();
    bitset_set(bitset, 1);
    g_assert_cmpint(bitset->low, ==, 1);
    bitset_set(bitset, 0);
    g_assert_cmpint(bitset->low, ==, 0);
    bitset_reset(bitset, 0);
    g_assert_cmpint(bitset->low, ==, 1);
    bitset_reset_all(bitset);
    g_assert_cmpint(bitset->high, ==, bitset->low);
}

void test_bitset_equals()
{
    BitSet *bitset1 = bitset_create();

    bitset_assign(bitset1, 0, true);
    bitset_assign(bitset1, 1, false);
    bitset_assign(bitset1, 2, false);
    bitset_assign(bitset1, 3, true);

    BitSet *bitset2 = bitset_create();
    bitset_assign(bitset2, 0, true);
    bitset_assign(bitset2, 1, false);
    bitset_assign(bitset2, 2, false);
    bitset_assign(bitset2, 3, true);
    bitset_assign(bitset2, 4, false);

    BitSet *bitset3 = bitset_create();
    bitset_assign(bitset3, 0, true);
    bitset_assign(bitset3, 1, false);
    bitset_assign(bitset3, 2, false);
    bitset_assign(bitset3, 3, false);
    bitset_assign(bitset3, 4, true);

    g_assert_cmpint(bitset_equals(bitset1, bitset2), ==, true);
    g_assert_cmpint(bitset_equals(bitset1, bitset3), ==, false);
}

void test_bitset_flip()
{
    BitSet *bitset = bitset_create();

    bitset_assign(bitset, 0, true);
    bitset_assign(bitset, 1, false);
    bitset_assign(bitset, 2, false);
    bitset_assign(bitset, 3, true);
    bitset_assign(bitset, 4, true);

    bitset_flip(bitset, bitset->low, bitset->high+1);

    g_assert_cmpint(bitset_test(bitset, 0), ==, false);
    g_assert_cmpint(bitset_test(bitset, 1), ==, true);
    g_assert_cmpint(bitset_test(bitset, 2), ==, true);
    g_assert_cmpint(bitset_test(bitset, 3), ==, false);
    g_assert_cmpint(bitset_test(bitset, 4), ==, false);
}

void test_bitset_and()
{
    BitSet *bitset1 = bitset_create();
    BitSet *bitset2 = bitset_create();
    bitset_set(bitset1, 1);
    bitset_set(bitset2, 0);

    bitset_and(bitset1, bitset2);

    g_assert_cmpint(bitset_any(bitset1), ==, false);

    bitset_reset_all(bitset1);
    bitset_reset_all(bitset2);

    bitset_set(bitset1, 0);
    bitset_set(bitset2, 1);

    bitset_and(bitset1, bitset2);
    g_assert_cmpint(bitset_any(bitset1), ==, false);

    bitset_reset_all(bitset1);
    bitset_reset_all(bitset2);

    bitset_set(bitset1, 1);
    bitset_set(bitset2, 2);
    bitset_set(bitset2, 3);
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
    add_test(test_bitset_to_string);
    add_test(test_bitset_or);
    add_test(test_bitset_xor);
    add_test(test_bitset_count);
    add_test(test_bitset_high);
    add_test(test_bitset_low);
    add_test(test_bitset_flip);
    add_test(test_bitset_equals);
    add_test(test_bitset_and);
    add_test(test_copy_bitset);
    add_test(test_bitset_assign);
    /* add_test(test_from_value); */

    return g_test_run();
}
