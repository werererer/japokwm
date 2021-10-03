#include "bitset/bitset.h"
#include "utils/coreUtils.h"
#include <GLES2/gl2.h>
#include <assert.h>
#include <wlr/util/log.h>

/****************** INTERFACE ******************/

BitSet *bitset_create(size_t minimum_number_of_bits) {
    BitSet *bitset = calloc(1, sizeof(*bitset));
    size_t number_of_bytes = BITS_TO_BYTES(minimum_number_of_bits);

    bitset->bits = g_ptr_array_sized_new(number_of_bytes);

    bitset->size = minimum_number_of_bits;
    for (size_t byte = 0; byte < number_of_bytes; ++byte) {
        bitset_grow(bitset);
    }

    return bitset;
}

static void *copy_int(const void *src_ptr, void *data)
{
    int *src = (int *)src_ptr;
    int *dest = calloc(1, sizeof(*dest));
    memcpy(dest, src, sizeof(uint8_t));
    return dest;
}

BitSet* bitset_copy(BitSet* source) {
    assert(source != NULL);

    BitSet *destination = malloc(sizeof(BitSet));
    destination->bits = g_ptr_array_copy(source->bits, copy_int, NULL);
    destination->size = source->size;

    return destination;
}

void bitset_assign_bitset(BitSet** dest, BitSet* source)
{
    if (*dest == source) {
        return;
    }
    // TODO: this can be optimized
    bitset_destroy(*dest);
    *dest = bitset_copy(source);
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
    g_ptr_array_unref(bitset->bits);
    free(bitset);
}

BitSet *bitset_from_value(uint64_t value) {
    BitSet *bitset = bitset_create(64);
    for (size_t bit = 0; bit < 64; ++bit) {
        bitset_assign(bitset, bit, LAST_BIT(value));
        value <<= 1;
    }

    return bitset;
}

int bitset_equals(BitSet* bitset1, BitSet* bitset2)
{
    BitSet *bitset = bitset_copy(bitset1);

    for (int bit = 0; bit < bitset->size; bit++) {
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
    size_t smaller_size;

    assert(destination != NULL);
    assert(source != NULL);

    if (destination == NULL) return BITSET_ERROR;
    if (source == NULL) return BITSET_ERROR;

    smaller_size = MIN(destination->size, source->size);

    for (size_t bit = 0; bit < smaller_size; bit++) {
        bool first = bitset_test(destination, bit);
        const bool second = bitset_test(source, bit);

        bit_operator(&first, &second);
        bitset_assign(destination, bit, first);
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

    for (int byte_idx = 0; byte_idx < bitset->bits->len; byte_idx++) {
        uint8_t* byte = g_ptr_array_index(bitset->bits, byte_idx);
        *byte = ~(*byte);
    }

    return BITSET_SUCCESS;
}

int bitset_set(BitSet* bitset, size_t index) {
    uint8_t* byte;
    if ((byte = byte_get(bitset, index)) == NULL) {
        return BITSET_ERROR;
    }

    *byte |= _bitset_index(index);

    return BITSET_SUCCESS;
}

int bitset_reset(BitSet* bitset, size_t index) {
    uint8_t* byte;
    if ((byte = byte_get(bitset, index)) == NULL) {
        return BITSET_ERROR;
    }

    *byte &= ~(_bitset_index(index));

    return BITSET_SUCCESS;
}

int bitset_assign(BitSet* bitset, size_t index, bool value) {
    /*
     * I estimate that this is more efficient than the usual "reset, then OR in
     * the value" pattern
     */
    if (value) {
        return bitset_set(bitset, index);
    } else {
        return bitset_reset(bitset, index);
    }
}

int bitset_toggle(BitSet* bitset, size_t index) {
    uint8_t* byte;
    if ((byte = byte_get(bitset, index)) == NULL) {
        return BITSET_ERROR;
    }

    *byte ^= _bitset_index(index);

    return BITSET_SUCCESS;
}

int bitset_test(BitSet* bitset, size_t index) {
    const uint8_t* byte;
    if ((byte = byte_const_get(bitset, index)) == NULL) {
        return BITSET_ERROR;
    }

    return (*byte & _bitset_index(index)) != 0;
}

uint8_t* byte_get(BitSet* bitset, size_t index) {
    assert(bitset != NULL);
    if (bitset == NULL) return NULL;

    bitset_grow_to_size(bitset, index+1);
    int byte_index = _byte_index(index);
    return (uint8_t*)g_ptr_array_index(bitset->bits, byte_index);
}

const uint8_t* byte_const_get(BitSet* bitset, size_t index) {
    assert(bitset != NULL);
    if (bitset == NULL) return NULL;

    bitset_grow_to_size(bitset, index+1);
    int byte_index = _byte_index(index);
    return (const uint8_t*)g_ptr_array_index(bitset->bits, byte_index);
}

int bitset_msb(BitSet* bitset) {
    return bitset_test(bitset, 0);
}

int bitset_lsb(BitSet* bitset) {
    assert(bitset != NULL);
    if (bitset == NULL) return BITSET_ERROR;

    return bitset_test(bitset, bitset->size - 1);
}

int bitset_reset_all(BitSet* bitset) {
    return bitset_set_all_to_mask(bitset, 0);
}

int bitset_set_all(BitSet* bitset) {
    return bitset_set_all_to_mask(bitset, 0xff);
}

int bitset_set_all_to_mask(BitSet* bitset, uint8_t mask) {
    assert(bitset != NULL);
    if (bitset == NULL) return BITSET_ERROR;

    for (int byte_idx = 0; byte_idx < bitset->bits->len; byte_idx++) {
        uint8_t *byte = g_ptr_array_index(bitset->bits, byte_idx);
        *byte = mask;
    }

    return BITSET_SUCCESS;
}

void bitset_clear(BitSet* bitset) {
    assert(bitset != NULL);
    bitset->size = 0;

    list_clear(bitset->bits, free);
}

/* Size Management */
int bitset_push(BitSet* bitset, bool value) {
    if (value) {
        return bitset_push_one(bitset);
    } else {
        return bitset_push_zero(bitset);
    }
}

int bitset_push_one(BitSet* bitset) {
    if (_bitset_increment_size(bitset) == BITSET_ERROR) {
        return BITSET_ERROR;
    }

    if (bitset_set(bitset, bitset->size - 1) == BITSET_ERROR) {
        return BITSET_ERROR;
    }

    return BITSET_SUCCESS;
}

int bitset_push_zero(BitSet* bitset) {
    if (_bitset_increment_size(bitset) == BITSET_ERROR) {
        return BITSET_ERROR;
    }

    if (bitset_reset(bitset, bitset->size - 1) == BITSET_ERROR) {
        return BITSET_ERROR;
    }

    return BITSET_SUCCESS;
}

void bitset_pop(BitSet* bitset) {
    if (--bitset->size % 8 == 0) {
        bitset_shrink(bitset);
    }
}

/* Capacity Management */
void bitset_reserve(BitSet* bitset, size_t minimum_number_of_bits) {
    assert(bitset != NULL);

    int number_of_bytes = BITS_TO_BYTES(minimum_number_of_bits);
    while(bitset->bits->len < number_of_bytes) {
        bitset_grow(bitset);
    }
    while(bitset->bits->len > number_of_bytes) {
        bitset_shrink(bitset);
    }
    bitset->size = minimum_number_of_bits;
}


void bitset_grow_to_size(BitSet* bitset, size_t size)
{
    int number_of_bytes = BITS_TO_BYTES(size);
    if (bitset->size < size) {
        bitset->size = size;
    }
    while(bitset->bits->len < number_of_bytes) {
        bitset_grow(bitset);
    }
}

void bitset_grow(BitSet* bitset) {

    assert(bitset != NULL);

    /* ERROR/SUCCESS flags are the same */
    uint8_t *empty = calloc(1, sizeof(*empty));
    g_ptr_array_add(bitset->bits, empty);
}

void bitset_shrink(BitSet* bitset) {
    assert(bitset != NULL);

    uint8_t *byte = g_ptr_array_steal_index(bitset->bits, bitset->size-1);
    free(byte);
}

/* Information */
size_t bitset_capacity(const BitSet* bitset) {
    assert(bitset != NULL);
    return bitset->bits->len * 8;
}

size_t bitset_size_in_bytes(const BitSet* bitset) {
    assert(bitset != NULL);
    if (bitset == NULL) return false;
    return BITS_TO_BYTES(bitset->size);
}

int bitset_count(BitSet* bitset) {
    size_t count;

    assert(bitset != NULL);
    if (bitset == NULL) return BITSET_ERROR;

    count = 0;
    for (int byte_idx = 0; byte_idx < bitset->bits->len; byte_idx++) {
        uint8_t *byte = g_ptr_array_index(bitset->bits, byte_idx);
        count += _byte_popcount(*byte);
    }

    return count;
}

int bitset_all(BitSet* bitset) {
    assert(bitset != NULL);
    if (bitset == NULL) return BITSET_ERROR;

    for (int byte_idx = 0; byte_idx < bitset->bits->len; byte_idx++) {
        uint8_t *byte = g_ptr_array_index(bitset->bits, byte_idx);
        if (*byte != 0xff) {
            return false;
        }
    }

    return true;
}

int bitset_any(BitSet* bitset) {
    assert(bitset != NULL);
    if (bitset == NULL) return BITSET_ERROR;

    for (int byte_idx = 0; byte_idx < bitset->bits->len; byte_idx++) {
        uint8_t *byte = g_ptr_array_index(bitset->bits, byte_idx);
        if (*byte != 0) {
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
    for (int bit = 0; bit < bitset->size; bit++) {
        debug_print("%i\n", bitset_test(bitset, bit));
    }
}

/****************** PRIVATE ******************/

uint8_t _byte_popcount(uint8_t value) {
    value = (value & POPCOUNT_MASK1) + ((value >> 1) & POPCOUNT_MASK1);
    value = (value & POPCOUNT_MASK2) + ((value >> 2) & POPCOUNT_MASK2);
    value = (value & POPCOUNT_MASK3) + ((value >> 4) & POPCOUNT_MASK3);

    return value;
}

int _bitset_increment_size(BitSet* bitset) {
    assert(bitset != NULL);
    if (bitset == NULL) return BITSET_ERROR;

    if (bitset->size++ % 8 == 0) {
        uint8_t *empty = calloc(1, sizeof(*empty));
        g_ptr_array_add(bitset->bits, empty);
    }

    return BITSET_SUCCESS;
}

void _bit_and(bool* first, const bool* second) {
    *first &= *second;
}

void _bit_or(bool* first, const bool* second) {
    *first |= *second;
}

void _bit_xor(bool* first, const bool* second) {
    *first ^= *second;
}

size_t _byte_index(size_t index) {
    return index / 8;
}

uint8_t _bitset_index(size_t index) {
    return 1 << _bitset_offset(index);
}

uint8_t _bitset_offset(size_t index) {
    return 7 - (index % 8);
}
