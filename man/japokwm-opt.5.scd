japokwm-opt(5)

# NAME
	opt
# Description
	define options for japokwm
# Functions
	reload() - reload japokwm config file
# Methods
	add_mon_rule(rule)
		add rules for monitors

	add_rule(rule)
		add rules for windows

	bind_key(binding)
		add a keybinding
	
	set_layout_constraints(constraints)
		set the minimum ratio the slave can have
	
	set_master_constraints(constraints)
		set the minimum ratio the master can have
# Variables
	bool arrange_by_focus = false
		whether the layout should be arranged by focus. This can be
		useful for a fullscreen layout like monocle
	
	bool automatic_workspace_naming = false
		whether workspaces should be renamed to the active window in
		them.
