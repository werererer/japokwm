#include "bitset/bitset.h"
#include "utils/coreUtils.h"
#include <GLES2/gl2.h>
#include <assert.h>
#include <wlr/util/log.h>

/****************** INTERFACE ******************/

BitSet *bitset_create()
{
    BitSet *bitset = calloc(1, sizeof(*bitset));

    bitset->bytes = g_hash_table_new_full(g_int_hash, g_int_equal, free, free);

    return bitset;
}

static void append_bits(void *key_ptr, void *value_ptr, void *user_data)
{
    int *key = malloc(sizeof(*key));
    int *value = malloc(sizeof(*value));

    int key_v = *(int *)key_ptr;
    bool value_v = *(bool *)value_ptr;

    *key = key_v;
    *value = value_v;

    GHashTable *bits = user_data;
    g_hash_table_insert(bits, key, value);
}

BitSet* bitset_copy(BitSet* source)
{
    assert(source != NULL);

    BitSet *destination = malloc(sizeof(BitSet));

    destination->bytes = g_hash_table_new_full(g_int_hash, g_int_equal, free, free);
    g_hash_table_foreach(source->bytes, append_bits, destination->bytes);

    return destination;
}

void bitset_assign_bitset(BitSet** dest, BitSet* source)
{
    if (*dest == source) {
        return;
    }
    g_hash_table_remove_all((*dest)->bytes);
    g_hash_table_foreach(source->bytes, append_bits, (*dest)->bytes);
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
    BitSet *bitset = bitset_copy(bitset1);

    for (GList *iter = g_hash_table_get_values(bitset->bytes); iter; iter = iter->next) {
        int bit = *(int *)iter->data;
        bitset_toggle(bitset, bit);
    }

    bitset_and(bitset, bitset2);
    bool equals = bitset_none(bitset);
    bitset_destroy(bitset);
    return equals;
}

int bit_wise_operation(BitSet* destination,
                                                BitSet* source,
                                                bit_operator_t bit_operator) {
    assert(destination != NULL);
    assert(source != NULL);

    if (destination == NULL) return BITSET_ERROR;
    if (source == NULL) return BITSET_ERROR;

    for (GList *iter = g_hash_table_get_keys(source->bytes); iter; iter = iter->next) {
        size_t i = *(size_t *)iter->data;
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

int bitset_xor(BitSet* destination, BitSet* source) {
    return bit_wise_operation(destination, source, _bit_xor);
}

int bitset_flip(BitSet* bitset) {
    assert(bitset != NULL);
    if (bitset == NULL) return BITSET_ERROR;

    for (GList *iter = g_hash_table_get_values(bitset->bytes); iter; iter = iter->next) {
        int *byte = iter->data;
        *byte = !(*byte);
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

int bitset_assign(BitSet* bitset, size_t index, bool value) {
    if (value) {
        bitset_ensure_index_exist(bitset, index);
    }
    bool* byte = g_hash_table_lookup(bitset->bytes, &index);

    if (!byte)
        return BITSET_SUCCESS;
    *byte = value;
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
    for (GList *iter = g_hash_table_get_values(bitset->bytes); iter; iter = iter->next) {
        bool key = *(bool *)iter->data;
        printf("k_prev: %i\n", key);
    }
    int success = bitset_set_all_to_mask(bitset, 0);
    for (GList *iter = g_hash_table_get_values(bitset->bytes); iter; iter = iter->next) {
        bool key = *(bool *)iter->data;
        printf("k_end: %i\n", key);
    }
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
        uint8_t *byte = iter->data;
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

void print_bitset(BitSet *bitset)
{
    for (int i = 0; i < 8; i++) {
        bool bit = bitset_test(bitset, i);
        debug_print("%i\n", bit);
    }
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
