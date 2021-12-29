japokwm-opt(5)

# Type
	class Options
# Description
	define options for japokwm
# Functions
	reload() - reload japokwm config file
# Methods
	add_mon_rule(rule)
		add rules for monitors

	add_rule(rule)
		add rules for windows

	bind_key(string, function)
		add a keybinding where $1 represents the binding and $2 the function
		that will be called when the binding is pressed. Bindings are defined
		the following way:
		```
			<key>-<key>-...<key> <key>-<key>...<key> ... <key>-<key>...<key>
		```
		Examples:
			- "mod-q s a mod-d"
			- "mod-q mod-a g"
			- "mod-q mod-a"
			- "mod-q"
		The simplest keybinding is the last one and means that if you press mod
		and q at the same time you the function will be called.
		Keybindings using a space inbetween blocks are chained keybindings. That
		means in the keybinding mod-q mod-a you have to press first mod-q and
		then mod-a.
		If 2 Keybindings are defined where one has a longer chain than the other
		one and the other being a subset of the first one you can execute the
		other one by waiting a certain time when the requirement is fullfilled.
		For example if you have mod-q mod-a and mod-a defined pressing mod-a and
		waiting a certain amount of time will execute mod-q.
	
	set_layout_constraints(constraints)
		set the minimum ratio the slave can have
	
	set_master_constraints(constraints)
		set the minimum ratio the master can have
# Variables
	bool arrange_by_focus = false
		whether the layout should be arranged by focus. This can be
		useful for a fullscreen layouts like monocle
	
	bool automatic_tag_naming = false
		whether tags should be renamed to the active window in
		them.
	
	color border_color = "Color.red"
		the color of the border of the windows

	int border_width = 1
		the width of the border of the windows
	
	string default_layout = "monocle"
		the default layout for the windowmanager
	
	function() entry_focus_position_function = nil
		the function is called when a new window is opened. It specifies
		the focus position of the new window in the layout.
	
	funciton() entry_position_function = nil
		the function is called when a new window is opened. It specifies the
		position of the new window in the layout.
	
	int float_border_width = 1
		the width of the border of floating windows

	color focus_color = "Color.blue"
		the color of the focused window
	
	enum wlr_edges hidden_edges = ""
		the edges that should be hidden when the window is on the edge
		of the screen.
	
	int inner_gaps = 0
		the gaps between the windows.

	int mod = 1
		the modifier that replaces "m" in a keybinding string.The values
		can be either:
		- 1: replacing "m" with "Alt_L"
		- 2: replacing "m" with "Num_Lock"
		- 3: replacing "m" with "ISO_Level3_Shift"
		- 4: replacing "m" with "Super_L"
	
	int outer_gaps = 0
		the gaps between the windows and the edge of the screen.
	
	int repeat_delay = 500
		the delay between keypresses when a keybinding is repeated.
	
	int repeat_rate = 100
		the rate at which a keybinding is repeated.
	
	enum wlr_edges resize_direction = "left"
		the direction in which the layout resizes.
	
	color root_color = new Color(0.3, 0.3, 0.3, 1.0)
		the color of root.
	
	bool sloppy_focus = false
		if true the focus follows the mouse.
	
	bool smart_hidden_edges = false
		if true the edges on the edge of the screen are hidden.
	
	string[] tags = ["1", "2", "3", "4", "5", "6", "7", "8", "9"]
		the names of the tags.
