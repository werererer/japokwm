japokwm-info(5)

# NAME
	info
# Description
	Access information about the window manager.
# Functions
	layout get_active_layout()
		Returns the name of the currently active layout.
	container get_container_under_cursor()
		Returns the container under the cursor.
	workspace get_next_empty_workspace()
		Returns the name of the next empty workspace.
	int get_this_container_count()
		Returns the number of containers in the current workspace.
	bool is_container_not_in_limit(container)
		Returns true if the container $1 is not too small.
	bool is_container_not_in_master_limit(container)
		Returns true if the container $1 is not too small.
	bool is_keycombo(keycombo)
		Returns true if the keycombo $1 is pressed.
