#include "bitset/bitset.h"
#include "utils/vector.h"
#include "utils/coreUtils.h"
#include <assert.h>

/****************** INTERFACE ******************/

int bitset_setup(BitSet* bitset, size_t minimum_number_of_bits) {
    size_t number_of_bytes;

    assert(bitset != NULL);
    if (bitset == NULL) return BITSET_ERROR;

    number_of_bytes = BITS_TO_BYTES(minimum_number_of_bits);

    // clang-format off
    if (vector_setup(&bitset->bits,
                                     number_of_bytes,
                                     sizeof(uint8_t)) == BITSET_ERROR) {
        return BITSET_ERROR;
    }
    // clang-format on

    bitset->size = minimum_number_of_bits;
    for (size_t byte = 0; byte < number_of_bytes; ++byte) {
        bitset_grow(bitset);
    }

    return BITSET_SUCCESS;
}

int bitset_copy(BitSet* destination, BitSet* source) {
    assert(destination != NULL);
    assert(source != NULL);

    if (destination == NULL) return BITSET_ERROR;
    if (source == NULL) return BITSET_ERROR;

    if (vector_copy(&destination->bits, &source->bits) == VECTOR_ERROR) {
        return BITSET_ERROR;
    }
    destination->size = source->size;

    return BITSET_SUCCESS;
}

int bitset_move(BitSet* destination, BitSet* source) {
    assert(destination != NULL);
    assert(source != NULL);

    if (destination == NULL) return BITSET_ERROR;
    if (source == NULL) return BITSET_ERROR;

    if (vector_move(&destination->bits, &source->bits) == VECTOR_ERROR) {
        return BITSET_ERROR;
    }
    destination->size = source->size;

    return BITSET_SUCCESS;
}

int bitset_swap(BitSet* destination, BitSet* source) {
    assert(destination != NULL);
    assert(source != NULL);

    if (destination == NULL) return BITSET_ERROR;
    if (source == NULL) return BITSET_ERROR;

    if (vector_swap(&destination->bits, &source->bits) == VECTOR_ERROR) {
        return BITSET_ERROR;
    }
    _vector_swap(&destination->size, &source->size);

    return BITSET_SUCCESS;
}

int bitset_destroy(BitSet* bitset) {
    if (vector_destroy(&bitset->bits) == VECTOR_ERROR) {
        return BITSET_ERROR;
    }
    return BITSET_SUCCESS;
}

BitSet bitset_from_value(uint64_t value) {
    BitSet bitset;
    bitset_setup(&bitset, 64);
    for (size_t bit = 0; bit < 64; ++bit) {
        bitset_assign(&bitset, bit, LAST_BIT(value));
        value <<= 1;
    }

    return bitset;
}

int bitset_equals(BitSet* bitset1, BitSet* bitset2)
{
    BitSet bitset;
    bitset_copy(&bitset, bitset1);

    for (int i = 0; i < bitset.size; i++) {
        bitset_toggle(&bitset, i);
    }

    bitset_and(&bitset, bitset2);
    return bitset_none(&bitset);
}

int bit_wise_operation(BitSet* destination,
                                                const BitSet* source,
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

int bitset_and(BitSet* destination, const BitSet* source) {
    return bit_wise_operation(destination, source, _bit_and);
}

int bitset_or(BitSet* destination, const BitSet* source) {
    return bit_wise_operation(destination, source, _bit_or);
}

int bitset_xor(BitSet* destination, const BitSet* source) {
    return bit_wise_operation(destination, source, _bit_xor);
}

int bitset_flip(BitSet* bitset) {
    assert(bitset != NULL);
    if (bitset == NULL) return BITSET_ERROR;

    VECTOR_FOR_EACH(&bitset->bits, iterator) {
        uint8_t* byte = iterator_get(&iterator);
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

int bitset_test(const BitSet* bitset, size_t index) {
    const uint8_t* byte;
    if ((byte = byte_const_get(bitset, index)) == NULL) {
        return BITSET_ERROR;
    }

    return (*byte & _bitset_index(index)) != 0;
}

uint8_t* byte_get(BitSet* bitset, size_t index) {
    assert(bitset != NULL);
    if (bitset == NULL) return NULL;

    return (uint8_t*)vector_get(&bitset->bits, _byte_index(index));
}

const uint8_t* byte_const_get(const BitSet* bitset, size_t index) {
    assert(bitset != NULL);
    if (bitset == NULL) return NULL;

    return (const uint8_t*)vector_const_get(&bitset->bits, _byte_index(index));
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

    VECTOR_FOR_EACH(&bitset->bits, byte) {
        ITERATOR_GET_AS(uint8_t, &byte) = mask;
    }

    return BITSET_SUCCESS;
}

int bitset_clear(BitSet* bitset) {
    assert(bitset != NULL);
    if (bitset == NULL) return BITSET_ERROR;
    bitset->size = 0;

    return vector_clear(&bitset->bits);
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

int bitset_pop(BitSet* bitset) {
    if (--bitset->size % 8 == 0) {
        return bitset_shrink(bitset);
    }
    return BITSET_SUCCESS;
}

/* Capacity Management */
int bitset_reserve(BitSet* bitset, size_t minimum_number_of_bits) {
    assert(bitset != NULL);
    if (bitset == NULL) return BITSET_ERROR;

    /* ERROR/SUCCESS flags are the same */
    return vector_reserve(&bitset->bits, BITS_TO_BYTES(minimum_number_of_bits));
}

int bitset_grow(BitSet* bitset) {
    uint8_t empty = 0;

    assert(bitset != NULL);
    if (bitset == NULL) return BITSET_ERROR;

    /* ERROR/SUCCESS flags are the same */
    return vector_push_back(&bitset->bits, &empty);
}

int bitset_shrink(BitSet* bitset) {
    assert(bitset != NULL);
    if (bitset == NULL) return BITSET_ERROR;

    /* ERROR/SUCCESS flags are the same */
    return vector_pop_back(&bitset->bits);
}

/* Information */
bool bitset_is_initialized(const BitSet* bitset) {
    assert(bitset != NULL);
    if (bitset == NULL) return false;
    return vector_is_initialized(&bitset->bits);
}

size_t bitset_capacity(const BitSet* bitset) {
    assert(bitset != NULL);
    if (bitset == NULL) return false;
    return bitset->bits.size * 8;
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
    VECTOR_FOR_EACH(&bitset->bits, byte) {
        count += _byte_popcount(ITERATOR_GET_AS(uint8_t, &byte));
    }

    return count;
}

int bitset_all(BitSet* bitset) {
    assert(bitset != NULL);
    if (bitset == NULL) return BITSET_ERROR;

    VECTOR_FOR_EACH(&bitset->bits, byte) {
        if (ITERATOR_GET_AS(uint8_t, &byte) != 0xff) {
            return false;
        }
    }

    return true;
}

int bitset_any(BitSet* bitset) {
    assert(bitset != NULL);
    if (bitset == NULL) return BITSET_ERROR;

    VECTOR_FOR_EACH(&bitset->bits, byte) {
        if (ITERATOR_GET_AS(uint8_t, &byte) != 0) {
            return true;
        }
    }

    return false;
}

int bitset_none(BitSet* bitset) {
    assert(bitset != NULL);
    if (bitset == NULL) return BITSET_ERROR;

    VECTOR_FOR_EACH(&bitset->bits, byte) {
        if (ITERATOR_GET_AS(uint8_t, &byte) != 0) {
            return false;
        }
    }

    return true;
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
        uint8_t empty = 0;
        if (vector_push_back(&bitset->bits, &empty) == VECTOR_ERROR) {
            return BITSET_ERROR;
        }
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
