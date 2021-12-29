japokwm(5)

# DESCRIPTION
A japokwm configuration file is a lua file that is executed by japokwm. You can
use built-in static methods with it to change the behavior of your window manager and
change various settings. An example config is likely present in
/etc/japokwm/init.lua for you to check out.

Japokwm uses a oop system to order the features. That means there are multiple
classes and objectes provided by the windowmanager.

# GLOBAL CONFIG FILES
japokwm searches for the *init.lua* file in the following locations, in this order:
. $XDG_CONFIG_HOME/japokwm/init.lua
. ~/.config/japokwm/init.lua (suggested location)
. /etc/japokwm/init.lua

# LAYOUT CONFIG FILES
All layouts are searched in the following locations:
. $XDG_CONFIG_HOME/japokwm/layouts/
. ~/.config/japokwm/layouts/ (suggested location)
. /etc/japokwm/layouts/
A layout must be in a directory holding the name of the layout. In there you
have to place a init.lua. You can find examples in /etc/japokwm/layouts/ or man
1 japokwm.
That means they have to be of the form: <layout name>/init.lua
e.g.: ~/.config/japokwm/layouts/foo/init.lua

# ACCESS FEATURES
We devide japokwm config files into 2 types. layout local and global config
files. They differ in the following ways:
- Global config are executed once per per reload. Starting the window manager ++
also counts as a reload.
- Layout local config files are executed every time a layout is loaded
- The objects that are accessible differ:
	- Layout local config files:
		- event
			- object of class Event
		- opt
			- object of class Options
		- layout
			- object of class Layout
	- Global config files:
		- event
			- object of class Event
		- opt
			- object of class Options
		- layout
			- object of class Layout
	The objects opt and layout are either layout dependent or global depending
on the location.

Another way to access things is through static methods and static variables of
certain classes. For example, the Container class has a static variable called
focused. With this you can access the focused container. e.g.:
```
-- using a static variable
local focused_container = Container.focused
-- using a static method
local color_green = Color.new(0.0, 1.0, 0.0, 1.0)
```

# HOW TO READ DOCUMENTATION
The best way is to think about what you want to do and then looking for classes
which sound most useful. Just reading through the documentation may be too
boring and you may remember nothing.

# TERMINOLOGY
*container* ++
an array consisting of 4 floats where they represent the relative x, y, ++
	width and height respectively

*relative x/y/width/height* ++
when x = 0 you are at the left side of the screen. At x = 1 you are on ++
	the right side. At x = 0.5 you are at half the screen ++
	when width = 0 it is literally 0. When it is 1 it is as wide as the ++
	current monitor. When width = 0.5 it means half the width of the monitor

*root* ++
everything you see when you don't have a window open

*window* ++
an alias for container used *only* throughout the documentation

*rule* ++
TODO

# LIST OF INTERESTING FEATURES THAT ARE HARD TO FIND ABOUT
- scratchpad
- monitor/container rules
- japokmsg see man 1 japokmsg
