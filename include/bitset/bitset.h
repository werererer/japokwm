#ifndef BITSET_H
#define BITSET_H

/* this data struct acts as an infinite list like you may know from haskell. We
 * need this data struct so that the user can create new workspaces lazily.*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <glib.h>

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
    GHashTable *bytes;
    // you should set it to the parent
    void *data;
} BitSet;

/****************** INTERFACE ******************/

/* Setup */
BitSet *bitset_create();

BitSet* bitset_copy(BitSet* source);
// both dest and source must be initialized
void bitset_assign_bitset(BitSet** dest, BitSet* source);
// note the it reverses the bitset from the bitset up to end-1 because it is
// most of the time nicer to write according to python
void bitset_reverse(BitSet *bitset, int start, int end);
int bitset_swap(BitSet* destination, BitSet* source);

void bitset_destroy(BitSet* bitset);

/* Factory */
BitSet *bitset_from_value(uint64_t value);
BitSet *bitset_from_value_reversed(uint64_t value);

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
const bool byte_const_get(BitSet* bitset, size_t index);
bool* byte_get(BitSet* bitset, size_t index);

int bitset_msb(BitSet* bitset);

int bitset_reset_all(BitSet* bitset);
int bitset_set_all(BitSet* bitset);
int bitset_set_all_to_mask(BitSet* bitset, uint8_t mask);
void bitset_clear(BitSet* bitset);

/* Information */
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
#define FIRST_BIT(value) value & 1

#define CEIL(x) ((x == ((int)(x))) ? x : ((int)(x)) + 1)
#define BITS_TO_BYTES(bits) CEIL((bits) / 8.0)
#define DEFAULT_ALLOCATED_BYTES 1
#define DEFAULT_NUMBER_OF_BITS 8

void _bit_and(bool* first, const bool* second);
void _bit_or(bool* first, const bool* second);
void _bit_xor(bool* first, const bool* second);

#endif /* BITSET_H */
