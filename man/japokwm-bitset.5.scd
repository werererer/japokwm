japokwm-bitset(5)

# NAME
	bitset
# Description
	A bitset is an array of bits that has the size of infinity. You can also
	perform bitwise operations on bitsets.
# Operations
	; TODO: test this
	a[i]
		Returns the ith bit of bitset a.
	
	a[i] = b
		Sets the ith bit of bitset a to b.
	
	a ~ b
		Performs bitwise XOR on the bitsets a and b.
	
	a & b
		Performs bitwise AND on the bitsets a and b.
	
	a | b
		Performs bitwise OR on the bitsets a and b.
	
	not a
		Performs bitwise NOT on the bitset a.
# Functions
	BitSet new()
		Creates a new bitset.
# Methods
	BitSet \_xor(BitSet)
		Performs a bitwise XOR operation on self and $1.
	
	BitSet \_and(BitSet)
		Performs a bitwise AND operation on self and $1.
	
	BitSet \_or(BitSet)
		Performs a bitwise OR operation on self and $1.
	
	BitSet \_not()
		Performs a bitwise NOT operation on self.
# Variables
