japokwm-ring_buffer(5)

# Type
	class Ring
# Description
	A ring buffer is a fixed-size buffer that stores a sequence of strings.
# Static Methods
	Ring new(string[])
		Create a new ring buffer.
# Methods
	string get(int)
		Get the string at the current position.
	void next()
		Move the current position to the next position. And return the
		string at that position.
	void prev()
		Move the current position to the previous position. And return
		the string at that position.
