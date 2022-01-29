japokwm-tag(5)

# Type
	class Tag
# Description
	The tag is an extension of workspaces. You can select select multiple
	tags at the same tame.
# Static Methods
	Tag get(int)
		Get the tag at the index $1.
	Tag get_next_empty(int, Direction)
		Get the next empty tag after the Tag at $1. In the direction at
		$2.
# Methods
	void get_id()
		Get the id of the tag.
	void swap(Tag)
		Swap the tag with the tag at $1.
	void swap_smart(Tag)
		Swap the tag with the tag at $1 smartly.
	void toggle_bars()
		Toggle the visibility of the bars.
# Variables
	bars bars
		The bars of the tag.
	Bitset tags
		The visible tags from the tag.
	Focus_set focus_set
		The focus set of the windowmanager.
	Container[] focus_stack
		The focus stack of the windowmanager.
	layout layout
		The layout of this Tag.
	layout previous_layout
		The previous layout of this Tag.
	Container[] stack
		The stack of this Tag.
	Focus_Set visible_focus_set
		The visible focus set of this Tag.
	Container[] visible_focus_stack
		The visible focus stack of this Tag.
	Container[] visible_stack
		The visible stack of this Tag.
