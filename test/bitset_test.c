#include <stdio.h>
#include <glib.h>

#include "bitset/bitset.h"

void test_bitset()
{
    BitSet *bitset = bitset_create();
    int bit = bitset_test(bitset, 0);
    g_assert_cmpint(bit, ==, 0);
}

void test_move_bitset()
{
    /* BitSet *bitset1 = bitset_create(8); */
    /* BitSet *bitset2 = bitset_create(8); */
    /* bitset_move(bitset1, bitset2); */
    /* int bit = bitset_test(bitset1, 0); */
    /* g_assert_cmpint(bit, ==, 0); */
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
    add_test(test_move_bitset);
    add_test(test_copy_bitset);
    add_test(test_bitset_assign);
    /* add_test(test_from_value); */

    return g_test_run();
}
