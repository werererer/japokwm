japokwm-action(5)

# Type
	class Action
# Description
	A collection of functions to alter the state of the windowmanager.
# Static Methods
	void arrange()
		Forces the windowmanager to arrange the windows now.
	
	void create_output()
		Creates a new output.
	
	void deep_copy(table)
		Creates a deep copy of the table.
	
	void exec(string) ++
void exec(string, function(output))
		Executes the string in the shell asynchronously. If $2 is given, the
		function is called with the output of the command as argument
		synchronized.
	
	void focus_on_hidden_stack(int)
		Focuses the window at the hidden stack in position $1.
	
	void focus_on_stack(int)
		Focuses the window at the stack in position $1.
	
	; TODO: is this function really needed?
	void move_resize(int)
		??? (not documented)

	void move_to_scratchpad(container)
		Moves the container to the scratchpad.
	
	void resize_main(float)
		resizes the active layout by $1 percent in the resize direction
		given by the layout.
	
	void show_scratchpad()
		Shows the next container in the scratchpad.
	
	; TODO: is this function really needed?
	void start_keycombo(string)
		??? (not documented)
	
	void swap_on_hidden_stack(int)
		Swaps the window at the hidden stack in position $1.

	void toggle_all_bars()
		Toggles the visibility of all bars in all tags.
	
	void toggle_tags()
		Toggles the previously selected tags.
	
	; TODO: is this function really needed?
	void toggle_view()
		??? (not documented)
	
	void toggle_tag()
		Toggles the previously selected tag.
	
	void view(tag)
		Views the tag $1.
	
	void view_or_tag(tag)
		Views the tag $1 or selects the tag if in a keycombo.
	
	void zoom()
		dwm like zoom. That is, if the currently focused window is not
		the master window then it will become it. If it is the master
		window however the window at position 2 will become the master
		window instead.

# Methods
# Variables
