japokwm(5)

# NAME

japokwm - tiling made easy

# DESCRIPTION
A japokwm configuration file is a lua file that is executed by japokwm. You can
use built-in functions with it to change the behavior of your window manager and
change various settings. An example config is likely present in
/etc/japokwm/init.lua for you to check out.

# NAMESPACES
*action* (see japokwm-config-action(1))
	Execute an action
*config* (see japokwm-config-config(1))
	Configure certain properties or the behavior of the window manager
*container*
	Configure certain properties or the behavior of a container
*event*
	Bind a function to an event
*layout*
	Configure certain properties or the behavior of a layout
*l*
	Configure things locally to the layout

# ACTION
*arrange()* - Arrange containers to the given information

*decrease_nmaster()* - decrease the number of master windows

*exec()* - execute a shell command in a new pid

*focus_container(i)* - focus the container at i
	i: integer - absolute position on the stack

*focus_on_hidden_stack(i)* - replace current container with a hidden container
	- i: integer - relative position on the hidden stack

*focus_on_stack(i)* - focuses the container at relative i
	i: integer - relative position on the stack

*increase_nmaster()* - increase the number of master windows

*kill()* - kill the focused window

*load_layout(l)* - load a layout
	l: Array{string, string}

*load_layout_in_set(set, i)* - load a layout in the layout set
	s: string - the name of the set
	i: position - the layout

*load_next_layout_in_set(set)* - load the next layout in the layout set
	s: string - the name of the set

*load_prev_layout_in_set(set)* - load the previous layout in the layout set
	s: string - the name of the set

*move_container_to_workspace(i)* - move the selected container to the workspace i
	i: integer

*move_resize(e)* - move and resize the selected container to the mouse
	e: enum given by info.cursor.mode

*quit()* - quit the window manager

*repush()* - push the container into the master area

*resize_main(f)* - resize the master area
	f: number - relative size (between 0 and 1)

*set_floating(b)* - set the selected container's floating status

*set_nmaster(i)* - set the amount of master windows
	i: integer - the number of master windows

*swap_workspace(i, i2)* - swap the workspace i with the workspace i2
	i: integer
	i2: integer


*toggle_consider_layer_shell()* - toggles layer_shell bars

*toggle_floating()* - toggles if current container is floating

*toggle_layout()* - toggle previous layout

*toggle_workspace()* - toggle previous workspace
*view(i)* - view the ith workspace
	i: integer

*zoom()* - push the current container to the main position

# CONTAINER
*container_set_sticky(b)* - make container sticky
	b: boolean

*container_set_ratio(f)* - set the ratio of the container
	f: number

# EVENT
*set_update_function(function(n) ... end)* - set the update function
	n: integer

*set_create_container_function()*

## LAYOUT
*set()* - 

# INFO
*get_this_container_count()* - 

*this_container_position()* - 

*get_next_empty_workspace()* - 

*get_workspace()* - 

*get_container_under_cursor()* - 

*is_container_not_in_limit()* - 

*is_container_not_in_master_limit()* - 

# CONFIG

*get_nmaster()* - 

*reload()* - 

*set_border_color()* - 

*set_buttons()* - 

*set_default_layout()* - 

*set_float_borderpx()* - 

*set_focus_color()* - 

*set_inner_gaps()* - 

*set_keybinds()* - 

*set_layout_constraints()* - 

*set_layouts()* - 

*set_master_constraints()* - 

*set_master_layout_data()* - 

*set_mod()* - 

*set_monrules()* - 

*set_outer_gaps()* - 

*set_repeat_delay()* - 

*set_repeat_rate()* - 

*set_resize_data()* - 

*set_resize_direction()* - 

*set_root_color()* - 

*set_rules()* - 

*set_sloppy_focus()* - 

*set_tile_borderpx()* - 

*set_workspaces()* - 

# LOCAL
config beginning with l are considered local config and only apply for the
current layout

## LOCALCONFIG
*set_arrange_by_focus()* - 

*set_border_color()* - 

*set_tile_borderpx()* - 

*set_float_borderpx()* - 

*set_focus_color()* - 

*set_inner_gaps()* - 

*set_outer_gaps()* - 

*set_sloppy_focus()* - 

*set_layout_constraints()* - 

*set_master_constraints()* - 

*set_resize_direction()* - 

*set_master_layout_data()* - 

*set_resize_data()* - 

## LOCALEVENT
*set_update_function()* - 

*set_create_container_function()* - 

# SEE ALSO

*japokwm*(1)
