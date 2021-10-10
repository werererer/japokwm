#ifndef BITSET_H
#define BITSET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "glib.h"

/****************** DEFINTIIONS ******************/

#define VECTOR_ERROR -1
#define VECTOR_SUCCESS 0

#define BITSET_ERROR -1
#define BITSET_SUCCESS 0

#define BITSET_INITIALIZER \
    { VECTOR_INITIALIZER, 0 }

typedef void (*bit_operator_t)(bool*, const bool*);

/****************** STRUCTURES ******************/

typedef struct BitSet {
    GPtrArray *bytes;
    size_t size;
} BitSet;

/****************** INTERFACE ******************/

/* Setup */
BitSet *bitset_create();

BitSet* bitset_copy(BitSet* source);
// both dest and source must be initialized
void bitset_assign_bitset(BitSet** dest, BitSet* source);
int bitset_swap(BitSet* destination, BitSet* source);

void bitset_destroy(BitSet* bitset);

/* Factory */
BitSet *bitset_from_value(uint64_t value);

/* Logical Operations */
int byte_wise_operation(BitSet* destination,
                                                const BitSet* source,
                                                bit_operator_t byte_operation);

int bitset_and(BitSet* destination, BitSet* source);
int bitset_or(BitSet* destination, BitSet* source);
int bitset_xor(BitSet* destination, BitSet* source);
int bitset_flip(BitSet* bitset);

/* Access */
int bitset_set(BitSet* bitset, size_t index);
int bitset_reset(BitSet* bitset, size_t index);
int bitset_assign(BitSet* bitset, size_t index, bool value);
int bitset_toggle(BitSet* bitset, size_t index);

int bitset_test(BitSet* bitset, size_t index);
const uint8_t* byte_const_get(BitSet* bitset, size_t index);
uint8_t* byte_get(BitSet* bitset, size_t index);

int bitset_msb(BitSet* bitset);
int bitset_lsb(BitSet* bitset);

int bitset_reset_all(BitSet* bitset);
int bitset_set_all(BitSet* bitset);
int bitset_set_all_to_mask(BitSet* bitset, uint8_t mask);
void bitset_clear(BitSet* bitset);

/* Size Management */
int bitset_push(BitSet* bitset, bool value);
int bitset_push_one(BitSet* bitset);
int bitset_push_zero(BitSet* bitset);
void bitset_pop(BitSet* bitset);

/* Capacity Management */
void bitset_reserve(BitSet* bitset, size_t minimum_number_of_bits);
void bitset_grow_to_size(BitSet* bitset, size_t size);
void bitset_shrink_to_size(BitSet* bitset, size_t size);
void bitset_grow(BitSet* bitset);
void bitset_shrink(BitSet* bitset);

/* Information */
size_t bitset_capacity(const BitSet* bitset);
size_t bitset_size_in_bytes(const BitSet* bitset);

int bitset_count(BitSet* bitset);
int bitset_all(BitSet* bitset);
int bitset_any(BitSet* bitset);
int bitset_none(BitSet* bitset);

/* Debugging */
void print_bitset(BitSet *bitset);

/****************** PRIVATE ******************/

#define LAST_BIT_INDEX(value) ((sizeof(value) * 8) - 1)
#define LAST_BIT_MASK(value) (1ULL << LAST_BIT_INDEX(value))
#define LAST_BIT(value) \
    ((value & LAST_BIT_MASK(value)) >> LAST_BIT_INDEX(value))

#define CEIL(x) ((x == ((int)(x))) ? x : ((int)(x)) + 1)
#define BITS_TO_BYTES(bits) CEIL((bits) / 8.0)
#define DEFAULT_ALLOCATED_BYTES 1
#define DEFAULT_NUMBER_OF_BITS 8

/* Popcount Masks */
#define POPCOUNT_MASK1 0x55
#define POPCOUNT_MASK2 0x33
#define POPCOUNT_MASK3 0x0F

uint8_t _byte_popcount(uint8_t value);

int _bitset_increment_size(BitSet* bitset);

void _bit_and(bool* first, const bool* second);
void _bit_or(bool* first, const bool* second);
void _bit_xor(bool* first, const bool* second);

size_t _byte_index(size_t index);
uint8_t _bitset_index(size_t index);
uint8_t _bitset_offset(size_t index);

#endif /* BITSET_H */
