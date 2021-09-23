japokwm(5)

# NAME

japokwm - tiling made easy

# DESCRIPTION
A japokwm configuration file is a lua file that is executed by japokwm. You can
use built-in functions with it to change the behavior of your window manager and
change various settings. An example config is likely present in
/etc/japokwm/init.lua for you to check out.

# Terminology
*container* ++
	an array consisting of 4 floats where they represent the relative x, y, ++
	width and height respectively ++
*relative x/y/width/height* ++
	when x = 0 you are at the left side of the screen. At x = 1 you are on ++
	the right side. At x = 0.5 you are at half the screen ++
	when width = 0 it is literally 0. When it is 1 it is as wide as the ++
	current monitor. When width = 0.5 it means half the width of the monitor ++
*root*
	everything you see when you don't have a window open
*window* ++
	the container and the content you see in it

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
|  direction
:< an exist
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
:< array(string) The 1st argument represents the directory/name of the layout
:< {"main"} -- the directory/name is "main"
|  color
:< array(number, number, number, number) the arguments represents rgba
   respectively between 0..1
:< {0, 1, 1, 1} -- cyan
|  keybinding
:< array(string, function()) the arguments are the keys given by the string ++
   and the function representing the action that will be taken
:< {"M-S-e", function() print("test") end}
|  rule
:< array(string, string, function(n)) the arguments are class name and rule ++
   respectively. The class and name are obtained from the window and the rule ++
   is just a function that gets the container id. The function will be ++
   executed when the layout is rearrange.
:< {"anki", "", function() print("test") end}
|  monrule
:< array(string, function()) the arguments are name and rule
   respectively. The class and name are obtained from the window and the rule is
   just a function without arguments. The function will be executed
   when a layout is created.
:< {"", function() print("test") end}

# NAMESPACES
[[ *Namespace*
:[ *Description*
|  action
:  Execute an action
|  config
:  Configure certain properties or the behavior of the window manager
|  container
:  Configure certain properties or the behavior of a container
|  event
:  Bind a function to an event
|  layout
:  Configure certain properties or the behavior of a layout
|  l
:  Configure things locally to the layout

# EVENTS
[[ *Event name*
:[ *when*
:[ *signature*
:[ *Description*
|  on_start
:  once when the windowmanager starts
:  void function()
:  
|  on_focus
:  when a new window is focused
:  void function(int i)
:  i: container id
|  on_update
:  when the state of the layout is updated
:  void function(int i)
:  i: count of containers
|  on_create_container
:  when a container is created
:  void function(int i)
:  i: container id

# ACTION
*arrange()* ++
	Arrange containers to the given information

*create_output()* ++
	just create another output

*decrease_nmaster()* ++
	decrease the number of master windows

*exec(cmd)* ++
	execute a shell command in a new pid ++
<where> ++
	cmd: string - the command

*focus_container(i)* ++
	focus the container at i ++
<where> ++
	i: integer - position on the stack

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

*kill(i)* ++
	kill the window at position i
<where> ++
	i: integer - position on the stack

*load_layout(l)* ++
	load a layout ++
<where> ++
	l: string - represents the layout name

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

*move_to_scratchpad(i)* ++
	move the window at position i to the scratchpad
<where> ++
	i: integer - position on the stack

*quit()* ++
	quit the window manager

*repush(i, j)* ++
	push the container into the master area ++
<where> ++
	i: integer - container that needs to repushed ++
	j: integer - new position it will be pushed to

*resize_main(n)* ++
	resize the master area ++
<where> ++
	n: number - relative size (between 0 and 1)

*set_floating(b)* ++
	set the selected container's floating status ++
<where> ++
	b: boolean - status(true/false)

*set_nmaster(i)*
	set the amount of master windows ++
<where> ++
	i: integer - the number of master windows

*show_scratchpad()*
	show the first window on the scratchpad. If it is already visible hide it
	instead and move it to the end of the scratchpad.

*swap_workspace(i, i2)*
	swap the workspace i with the workspace i2 ++
<where> ++
	i: integer - workspace at i ++
	i2: integer - workspace at i2

*tag_view(i)*
	tag a view
<where> ++
	i: integer - an integer representing a binary number where each digit
	represents a workspace that will be toggled

*toggle_bars()* ++
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
	like dwm's zoom

# CONTAINER
*container_set_alpha(i, alpha)* ++
	set the opacity of the container ++
<where> ++
	i: integer - position of the container
	alpha: float - new alpha value of the container

*container_set_ratio(i, ratio)* ++
	set the ratio of the container ++
<where> ++
	i: integer - position of the container
	ratio: float - ratio of the container (if 0 it is interpreted as no ratio)

*container_set_sticky(i, sticky)* ++
	make container sticky ++
<where> ++
	i: integer - position of the container
	sticky: integer - should represent the binary number for the workspaces to
	set it sticky to. If it is 0 the container will be moved to the scratchpad

# EVENT
*add_listener(event_name, func)* ++
	add a add_listener to an event ++
<where> ++
	event_name: string - identifier of the event ++
	func: function(n) - the function that will be called on event specified by event_name ++
	<where> ++
		n: integer - n represents the amount of containers

# LAYOUT
*set(layout_data)* ++
	set layout ++
<where> ++
	layout_data: layout_data - layout_data for the layout

# INFO
*get_active_layout()* ++
	get the name of the layout currently active on the selected workspace
<return> ++
	string - name of the layout

*get_this_container_count()* ++
	get the amount of visible containers in this workspace
<return> ++
	integer - number of containers

*this_container_position()* ++
	get the position the container is at ++
<return> ++
	integer - position of the focused container

*get_nmaster()* ++
	get the number of master windows ++
<return> ++
	integer - number of master windows

*get_previous_layout()* ++
	get the name of the layout previously active on the selected workspace
<return> ++
	string - name of the layout

*get_root_area()* ++
	get the root area
<return> ++
	table(x, y, width, height) ++
<where> ++ 
	x: integer - x coordinate ++
	y: integer - y coordinate ++
	width: integer - width of the root area ++
	height: integer - height of the root area

*get_next_empty_workspace()* ++
	Get next workspaces not used by any window
<return> ++
	integer - workspace id of this empty workspace

*get_workspace()* ++
	Get the workspaces id of the current workspace
<return> ++
	integer - workspace id

*get_container_under_cursor()* ++
	get the container beneath the cursor ++
<return> ++
	integer - container id

*get_n_tiled()* ++
	get the amount of containers currently visible ++
<return> ++
	integer - amount of containers

*is_container_not_in_limit()* ++
	returns whether a container doesn't violate min/max_width/height of the
	constraints ++
<return> ++
	boolean - whether it is in limit

*is_container_not_in_master_limit()* ++
	returns whether a container doesn't violate min/max_width/height of the ++
	master constraints ++
<return> ++
	boolean - whether it is in limit

*stack_position_to_position(i)* ++
	converts a position on the container stack to a position
<where> ++
	i - the position of the container on the container stack
<return> ++
	i - the position of the container

# CONFIG
*add_mon_rule(rule)*
	add a rule for monitors
<where> ++
	rule: rule - 

*add_rule(rule)*
	add a rule for containers
<where> ++
	rule: rule - 

*bind_key(bind, func)*
	bind a keybinding to a lua function
<where> ++
	bind: keybinding - the binding that will execute the function
	func: function() - the function that will be executed

*create_layout_set(name, layouts)*
	create a new layout_set ++
<where> ++
	name: string - name of the layout set ++
	layouts: layout_info - layouts that belong to this set

*create_workspaces(names)*
	create or recreate all workspaces with an array of names for each ++
	workspace. The order of the workspace can be set by prefixing the names ++
	with: "%i:" where %i represents the position the workspace will be on. ++
<where> ++
	names: array(string)

*reload()* ++
	reload the config file

*set_arrange_by_focus(b)* ++
	if b is true windows will be tiled by means of the focus stack
<where> ++
	b: boolean

*set_automatic_workspace_naming(b)* ++
	if b is true workspaces will be automatically to the app-id of the focused 
	container on on that particular workspace
<where> ++
	b: boolean


*set_border_color(color)* ++
	set the border color of windows ++
<where> ++
	color: color

*set_default_layout(l)* ++
	set the default layout ++
<where> ++
	l: string - name/directory of the layout

*set_float_borderpx(f)* ++
	set the border width of floating windows in pixel ++
<where> ++
	f: border width in pixel

*set_focus_color(color)* ++
	set the color of the focused window ++
<where> ++
	color: color

*set_focus_color(color)* ++
	set the color of the focused window ++
<where> ++
	color: color

*set_hidden_edges(d)* ++
	set the directions edge borders will be hidden ++
<where> ++
	d: direction

*set_layout_constraints(min_width: a, max_width: b, min_height: c, max_height: d)* ++
	Set the minimum and maximum dimensions of resizing any window ++
<where> ++
	a, b, c, d: number - relative width/height

*set_master_constraints(min_width: a, max_width: b, min_height: c, max_height: d)* ++
	Set the minimum and maximum dimensions of resizing the master area ++
<where> ++
	a, b, c, d: number - relative width/height

*set_master_layout_data(data)* ++
	set the way the windows in the master area are tiled depending on nmaster ++
<where> ++
	data: layout_data

*set_mod(i)* ++
	set the modifier which is any number between 1 and 4. This causes the ++
	"mod" string to be replaced such that they correspond to the correct ++
	modifiers:

[[ Number
:< Mod
:< Literally
|< 1
:< Alt
:< "Alt_L"
|< 2
:< Number lock
:< "Num_Lock"
|< 3
:< AltGr
:< "ISO_Level3_Shift"
|< 4
:< Super
:< "Super_L"

<where> ++
	i: integer

*set_outer_gaps(i)* ++
	set how large the gap between all the windows and the root is ++
<where> ++
	i: integer - the size of those gaps in pixel

*set_repeat_delay(i)* ++
	how long do you need to wait before the keyboard starts to repeat in ++
<where> ++
	i: integer - i is the delay given in milliseconds

*set_repeat_rate(i)* ++
	how often will a character repeat ++
<where> ++
	i: integer - i is the repeat rate given in milliseconds

*set_resize_data(data)*
	set the resize direction of the layout ++
	; TODO improve this
<where> ++
	data: array(array(integer, ...))

*set_resize_direction(d)* ++
	set the resize direction of the layout ++
<where> ++
	d: direction

*set_resize_function(func)* ++
	the function that handles resizing windows in a layout
<where> ++
	func: function(layout_data, o_layout_data, resize_data, n, direction) - the
	function that will be called when you resize your containers ++
	<where> ++
		layout_data: layout_data - the data for the current layout ++
		o_layout_data: layout_data - the layout how it originally was ++ 
		resize_data: array(array(integer, ...)) - which layout_data_elements to
		include when resizing ++
		n: float - how much it should resize ++
		direction: direction - which directions to resize to

*set_combo_timeout(timout)* ++
	set timeout for keyboard combos ++
<where> ++
	timout: integer

*set_root_color(color)* ++
	set color of the root ++
<where> ++
	color: color

*set_sloppy_focus(b)* ++
	set whether to use sloppy focus or not. If sloppy focus is activated you ++
	will focus windows by hovering above them. ++
<where> ++
	b: boolean

*set_smart_hidden_edges(b)* ++
	if true edges are only hidden (see set_hidden_edges) if the number of ++
	containers in the current workspace <= 1 ++
<where> ++
	b: boolean

*set_tile_borderpx(i)* ++
	set the border width of tiled windows in pixel ++
<where> ++
	i: integer - border width in pixel

# MONITOR
*set_scale(n)* ++
	scale the monitor by n
<where> ++
	n: number - the percentage the monitor will be scaled e.g. 1 = 100%

*set_transform(e)* ++
	scale the monitor by n
<where> ++
	% TODO: what is this enum?
	e: WL_LIST_TRANSFORMATION - an enum provided by info.monitor.transform

# LOCAL
config beginning with l are considered local config and only apply for the
current layout. There are basically the same functions as usual but only some of
them can be used locally. Here is a list of available functions:

## CONFIG
set_arrange_by_focus ++
set_border_color ++
set_float_borderpx ++
set_focus_color ++
set_hidden_edges ++
set_inner_gaps ++
set_layout_constraints ++
set_master_constraints ++
set_master_layout_data ++
set_outer_gaps ++
set_resize_data ++
set_resize_direction ++
set_resize_function ++
set_sloppy_focus ++
set_smart_hidden_edges ++
set_tile_borderpx

## EVENT
add_listener

# SEE ALSO
*japokwm*(1)
