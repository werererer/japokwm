japokwm(5)

# NAME

japokwm - tiling made easy

# DESCRIPTION
A japokwm configuration file is a lua file that is executed by japokwm. You can
use built-in functions with it to change the behavior of your window manager and
change various settings. An example config is likely present in
/etc/japokwm/init.lua for you to check out.

# Types
Here are types listed which are used in Japokwm's config file. Those Types are
important because you won't understand the function definitions otherwise.
[[ *Types*
:< *Description*
:< *Example*
|  nil
:< basic lua nil
:< nil
|  string
:< basic lua string
:< "string"
|  number
:< basic lua number
:< 3.5
|  integer
:< basic lua number that must be a whole number
:< 3
|  function
:< basic lua number that must be a whole number
:< function() print("execute") end
|  boolean
:< basic lua boolean
:< true | false

In Japokwm multiple types of tables are used to different kinds of information.
We gave those special tables certain names so that you can tell them apart more
easily. To show the content of a table we write one as follows:

```
table(string: string, string: integer, ...)
```

This table consists out of elements consisting out of keys of type string and a
value of type string and elements consisting out of keys of type string and
values of type integers which can exist anywhere between 0 and infinity times
denoted by "..."

Tables without keys are called arrays and are denoted as follows:

```
array(string, integer, ...)
```

This is an array with a string, then an integer followed by any number of
integers

[[ *Types*
:< *Description*
:< *Example*
|  container
:< array(number, number, number, number) where each number represents x, y,
   width or height in the given order
:< {0.3, 0.1, 0.5, 0.7}
|  layout_data_element
:< array(container, ...)
:< {{0.3, 0.1, 0.5, 0.7}, {0.3, 0.1, 0.5, 0.7},}
|  layout_data
:< array(layout_data_element, ...) where the first layout_data_element will be
   used
:< { { {0.3, 0.1, 0.5, 0.7}, {0.3, 0.1, 0.5, 0.7}, }, { {0.3, 0.1, 0.5, 0.7}, {0.3, 0.1, 0.5, 0.7},} }
|  layout_info
:< array(string, string) the 1st argument represents the symbol of the layout
   the 2nd argument represents the directory/name of the layout
:< {"+", "main"} -- symbol is "+" and directory "main"


# NAMESPACES
*action*
	Execute an action
*config*
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
*arrange()* ++
	Arrange containers to the given information

*decrease_nmaster()* ++
	decrease the number of master windows

*exec(cmd)* ++
	execute a shell command in a new pid ++
<where> ++
	cmd: string - the command

*focus_container(i)* ++
	focus the container at i ++
<where> ++
	i: integer - absolute position on the stack

*focus_on_hidden_stack(i)* ++
	replace current container with a hidden container ++
<where> ++
	i: integer - relative position on the hidden stack

*focus_on_stack(i)* ++
	focuses the container at relative i ++
<where> ++
	i: integer - relative position on the stack

*increase_nmaster()* ++
	increase the number of master windows

*kill()* - kill the focused window

*load_layout(l)* ++
	load a layout ++
<where> ++
	l: table(a: string, b: string) where

*load_layout_in_set(set, i: position)* ++
	load a layout in the layout set ++
<where> ++
	set: string - the name of the set ++
	i: the layout

*load_next_layout_in_set(set)* ++
	load the next layout in the layout set ++
<where> ++
	set: the name of the set

*load_prev_layout_in_set(set)* ++
	load the previous layout in the layout set ++
<where> ++
	set: string - the name of the set

*move_container_to_workspace(i)* ++
	move the selected container to the workspace i ++
<where> ++
	i: integer

*move_resize(e)* ++
	move and resize the selected container to the mouse ++
<where> ++
	e: enum - given by info.cursor.mode

*quit()* ++
	quit the window manager

*repush()* ++
	push the container into the master area

*resize_main(f)* ++
	resize the master area ++
<where> ++
	f: number - relative size (between 0 and 1)

*set_floating(b)*
	set the selected container's floating status

*set_nmaster(i)*
	set the amount of master windows ++
<where> ++
	i: integer - the number of master windows

*swap_workspace(i, i2)*
	swap the workspace i with the workspace i2 ++
<where> ++
	i: integer ++
	i2: integer

*toggle_consider_layer_shell()* ++
	toggles layer_shell bars

*toggle_floating()* ++
	toggles if current container is floating

*toggle_layout()* ++
	toggle previous layout

*toggle_workspace()* ++
	toggle previous workspace

*view(i)* ++
	view the ith workspace ++
<where> ++
	i: integer - ith workspace

*zoom()* ++
	push the current container to the main position

# CONTAINER
*container_set_sticky(b)* ++
	make container sticky ++
<where> ++
	b: boolean

*container_set_ratio(f)* ++
	set the ratio of the container ++
<where> ++
	f: number

# EVENT
*set_update_function(function(n) ... end)* ++
	set the update function ++
<where> ++
	n: integer

*set_create_container_function()*

## LAYOUT
*set(layout_data)* ++
	set layout ++
<where> ++
	layout_data: layout_data

# INFO
*get_this_container_count()* ++
	get the amount of visible containers

*this_container_position()* ++
	returns the position the container is at

*get_nmaster() ++
	> integer* - Returns the number of master windows

*get_next_empty_workspace()* ++
	Get next workspaces not used by any window

*get_workspace()* ++
	Get the workspaces id of the current workspace

*get_container_under_cursor()*
	get the container beneath the cursor
<return>
	integer: container id

*is_container_not_in_limit()* - 

*is_container_not_in_master_limit()* - 

# CONFIG

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
