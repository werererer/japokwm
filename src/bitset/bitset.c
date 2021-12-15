#include "bitset/bitset.h"
#include <GLES2/gl2.h>
#include <assert.h>
#include <wlr/util/log.h>

#include "stringop.h"
#include "utils/stringUtils.h"
#include "utils/coreUtils.h"

/****************** INTERFACE ******************/

BitSet *bitset_create()
{
    BitSet *bitset = calloc(1, sizeof(*bitset));

    bitset->low = 0;
    bitset->high = 0;

    bitset->bytes = g_hash_table_new_full(g_int_hash, g_int_equal, free, free);

    return bitset;
}

static void append_bits(void *key_ptr, void *value_ptr, void *user_data)
{
    int key = *(int *)key_ptr;
    bool value = *(bool *)value_ptr;

    BitSet *destination = user_data;
    bitset_assign(destination, key, value);
}

BitSet* bitset_copy(BitSet* source)
{
    assert(source != NULL);

    BitSet *destination = bitset_create();

    destination->bytes = g_hash_table_new_full(g_int_hash, g_int_equal, free, free);
    g_hash_table_foreach(source->bytes, append_bits, destination);

    return destination;
}

void bitset_assign_bitset(BitSet** dest, BitSet* source)
{
    if (*dest == source) {
        return;
    }
    g_hash_table_remove_all((*dest)->bytes);
    g_hash_table_foreach(source->bytes, append_bits, *dest);
}

void bitset_reverse(BitSet *bitset, int start, int end)
{
    BitSet *bitset_tmp = bitset_copy(bitset);
    for (int i = start; i < end; i++) {
        int high = end - 1;
        int reverse_i = high - i;
        bool b = bitset_test(bitset_tmp, reverse_i);
        bitset_assign(bitset, i, b);
    }
    bitset_destroy(bitset_tmp);
}

int bitset_swap(BitSet* destination, BitSet* source) {
    // TODO: fix later
    /* assert(destination != NULL); */
    /* assert(source != NULL); */

    /* if (destination == NULL) return BITSET_ERROR; */
    /* if (source == NULL) return BITSET_ERROR; */

    /* if (vector_swap(destination->bits, source->bits) == VECTOR_ERROR) { */
    /*     return BITSET_ERROR; */
    /* } */
    /* _vector_swap(&destination->size, &source->size); */

    return BITSET_SUCCESS;
}

void bitset_destroy(BitSet* bitset) {
    g_hash_table_unref(bitset->bytes);
    free(bitset);
}

BitSet *bitset_from_value(uint64_t value) {
    BitSet *bitset = bitset_create();
    for (size_t bit = 0; bit < 64; ++bit) {
        bool v = LAST_BIT(value);
        bitset_assign(bitset, bit, v);
        value <<= 1;
    }

    return bitset;
}

BitSet *bitset_from_value_reversed(uint64_t value)
{
    BitSet *bitset = bitset_create();
    for (size_t bit = 0; bit < 64; ++bit) {
        bool v = FIRST_BIT(value);
        bitset_assign(bitset, bit, v);
        value >>= 1;
    }

    return bitset;
}

int bitset_equals(BitSet* bitset1, BitSet* bitset2)
{
    if (bitset1->low != bitset2->low)
        return false;
    if (bitset1->high != bitset2->high)
        return false;
    for (int i = bitset1->low; i <= bitset2->high; i++) {
        bool b1 = bitset_test(bitset1, i);
        bool b2 = bitset_test(bitset2, i);
        if (b1 != b2) {
            return false;
        }
    }
    return true;
}

int bit_wise_operation(BitSet* destination,
                                                BitSet* source,
                                                bit_operator_t bit_operator) {
    assert(destination != NULL);
    assert(source != NULL);

    if (destination == NULL) return BITSET_ERROR;
    if (source == NULL) return BITSET_ERROR;

    int low = MIN(destination->low, source->low);
    int high = MAX(destination->high, source->high);
    for (int i = low; i <= high; i++) {
        bool first = bitset_test(destination, i);
        const bool second = bitset_test(source, i);

        bit_operator(&first, &second);

        bitset_assign(destination, i, first);
    }

    return BITSET_SUCCESS;
}

int bitset_and(BitSet* destination, BitSet* source) {
    return bit_wise_operation(destination, source, _bit_and);
}

int bitset_or(BitSet* destination, BitSet* source) {
    return bit_wise_operation(destination, source, _bit_or);
}

int bitset_xor(BitSet* destination, BitSet* source)
{
    return bit_wise_operation(destination, source, _bit_xor);
}

int bitset_flip(BitSet* bitset, int start, int end)
{
    assert(bitset != NULL);
    if (bitset == NULL) return BITSET_ERROR;

    for (int i = start; i < end; i++) {
        bool byte = bitset_test(bitset, i);
        bitset_assign(bitset, i, !byte);
    }

    return BITSET_SUCCESS;
}

int bitset_set(BitSet* bitset, size_t index) {
    return bitset_assign(bitset, index, true);
}

int bitset_reset(BitSet* bitset, size_t index) {
    return bitset_assign(bitset, index, false);
}

static void bitset_ensure_index_exist(BitSet* bitset, size_t index)
{
    if (!g_hash_table_contains(bitset->bytes, &index)) {
        size_t *key = malloc(sizeof(*key));
        *key = index;
        bool *value = malloc(sizeof(*value));
        *value = false;
        g_hash_table_insert(bitset->bytes, key, value);
    }
}

static void bitset_set_new_high(BitSet *bitset, int updated_index)
{
    int start_value = MAX(updated_index, bitset->high);
    for (int i = start_value; i >= bitset->low; i--) {
        if (bitset_test(bitset, i)) {
            bitset->high = i;
            return;
        }
    }
    // else
    bitset->high = bitset->low;
}

static void bitset_set_new_low(BitSet *bitset, int updated_index)
{
    int start_value = MIN(updated_index, bitset->low);
    for (int i = start_value; i <= bitset->high; i++) {
        if (bitset_test(bitset, i)) {
            bitset->low = i;
            return;
        }
    }
    // else
    bitset->low = bitset->high;
}

int bitset_assign(BitSet* bitset, size_t index, bool value) {
    if (value) {
        // the default value of a bit is false so we don't need to ensure that
        // such an index exist if the value is indeed false. this is just for
        // optimization
        bitset_ensure_index_exist(bitset, index);
    }
    bool* byte = g_hash_table_lookup(bitset->bytes, &index);

    if (byte) {
        *byte = value;

        bitset_set_new_high(bitset, index);
        bitset_set_new_low(bitset, index);
    }
    // debug_print("new low: %i\n", bitset->low);
    return BITSET_SUCCESS;
}

int bitset_toggle(BitSet* bitset, size_t index) {
    bool byte = byte_const_get(bitset, index);

    bool v = !byte;
    bitset_assign(bitset, index, !v);

    return BITSET_SUCCESS;
}

int bitset_test(BitSet* bitset, size_t index) {
    bool byte = byte_const_get(bitset, index);

    return byte;
}

const bool byte_const_get(BitSet* bitset, size_t index) {
    assert(bitset != NULL);
    if (bitset == NULL) return NULL;

    const bool *byte = g_hash_table_lookup(bitset->bytes, &index);

    bool ret_value = false;
    if (byte) {
        ret_value = *byte;
    }
    return ret_value;
}

int bitset_msb(BitSet* bitset) {
    return bitset_test(bitset, 0);
}

int bitset_reset_all(BitSet* bitset) {
    int success = bitset_set_all_to_mask(bitset, 0);
    return success;
}

int bitset_set_all(BitSet* bitset) {
    return bitset_set_all_to_mask(bitset, 0xff);
}

int bitset_set_all_to_mask(BitSet* bitset, uint8_t mask) {
    assert(bitset != NULL);
    if (bitset == NULL) return BITSET_ERROR;

    const int byte_len = 8;
    for (int i = 0; i < byte_len; i++) {
        bool value = LAST_BIT(mask);
        bitset_assign(bitset, i, value);
        mask <<= 1;
    }

    return BITSET_SUCCESS;
}

void bitset_clear(BitSet* bitset) {
    assert(bitset != NULL);
    g_hash_table_remove_all(bitset->bytes);
}

int bitset_count(BitSet* bitset) {
    assert(bitset != NULL);
    if (bitset == NULL) return BITSET_ERROR;

    size_t count = 0;
    for (GList *iter = g_hash_table_get_values(bitset->bytes); iter; iter = iter->next) {
        bool *byte = iter->data;
        if (*byte) {
            count++;
        }
    }

    return count;
}

int bitset_all(BitSet* bitset) {
    assert(bitset != NULL);
    if (bitset == NULL) return BITSET_ERROR;

    for (GList *iter = g_hash_table_get_values(bitset->bytes); iter; iter = iter->next) {
        bool *byte = iter->data;
        if (!*byte) {
            return false;
        }
    }

    return true;
}

int bitset_any(BitSet* bitset) {
    assert(bitset != NULL);
    if (bitset == NULL) return BITSET_ERROR;

    for (GList *iter = g_hash_table_get_values(bitset->bytes); iter; iter = iter->next) {
        bool *byte = iter->data;
        if (*byte) {
            return true;
        }
    }

    return false;
}

int bitset_none(BitSet* bitset) {
    assert(bitset != NULL);
    if (bitset == NULL) return BITSET_ERROR;

    return !bitset_any(bitset);

    return true;
}

char *bitset_to_string(BitSet* bitset)
{
    assert(bitset != NULL);
    if (bitset == NULL) return NULL;

    char *str = strdup("");

    int byte_start = bitset->low / 8;
    int byte_end = byte_start + 8;

    append_string(&str, "(");
    const char str_integer[NUM_DIGITS];
    int_to_string((char *)str_integer, byte_start);
    append_string(&str, str_integer);
    append_string(&str, ")");

    for (int i = byte_start; i < byte_end; i++) {
        bool bit = bitset_test(bitset, i);
        char *bit_str = bit ? "1" : "0";
        append_string(&str, bit_str);
    }

    return str;
}

void print_bitset(BitSet *bitset)
{
    char *str = bitset_to_string(bitset);
    printf("%s\n", str);
}

/****************** PRIVATE ******************/

void _bit_and(bool* first, const bool* second) {
    *first &= *second;
}

void _bit_or(bool* first, const bool* second) {
    *first |= *second;
}

void _bit_xor(bool* first, const bool* second) {
    *first ^= *second;
}
