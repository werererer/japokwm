japokwm(5)

# DESCRIPTION
A japokwm configuration file is a lua file that is executed by japokwm. You can
use built-in static methods to change the behavior and the settings of your
window manager. An example config is likely present in /etc/japokwm/init.lua for
you to check out.

Japokwm uses an oop system to order features. That means there are multiple
classes and objectes provided by the window manager.

# OOP - OBJECT ORIENTED PROGRAMMING IN LUA
You should get familiar with the oop programming before continuing using the
window manager. Or try learning it side by side.

A *static method* in lua is called by the dot operator using the class name. That
means if we have the class Color, then we can call the static method new to
create any color in the following way:
```
local color_red = Color.new(1, 0, 0, 1)
```
This creates the color red. and saves it in the local variable color_red.

A *static variable* is called by the dot operator using the class name.
E.g. if we have the class Color, then we can get the predefined color red by
calling:
```
local color_red = Color.RED
```

A *variable* is called by the dot operator using an object. E.g. if we want
to change the alpha value of the focused container we do it like this:
```
local focused_container = Container.focused()
focused_container.alpha = 0.5
```
now it will be 50% transparent.

A *method* is called by the *dot operator* using the object and using the *same
object as the first argument*. If we want to kill the focused
container for example we can do it like this:
```
local focused_con = Container.focused
if focused_con then
	focused_con.kill(focused_con)
end
```
since this is clearly a waste of time and energy, lua gave us syntactic sugar
for it. Instead of the dot operator we can use the colon operator. When you use
the colon operator the first argument is the object itself:
```
local focused_con = Container.focused
if focused_con then
	focused_con:kill()
end
```
We use an if statement to check if the object is not nil.


# GLOBAL CONFIG FILES
All files named *init.lua* that are in either of these directories:
. $XDG_CONFIG_HOME/japokwm/init.lua
. ~/.config/japokwm/init.lua (suggested location)
. /etc/japokwm/init.lua
are called global config files. That means they have a global scope and get
special access to certain objects (See list at #Access Features).

# LAYOUT CONFIG FILES
All *init.lua* files describing a layout are layout local config files. These
files are typically placed in a directory that is in one of the following
directories(the first has the most priority):
. $XDG_CONFIG_HOME/japokwm/layouts/
. ~/.config/japokwm/layouts/ (suggested location)
. /etc/japokwm/layouts/
. /usr/local/share/japokwm/layouts
. /usr/share/japokwm/layouts
A layout must be in a directory holding the name of the layout. In there you
have to place a init.lua. You can find examples in /etc/japokwm/layouts/ or man
1 japokwm.++
That means they have to be of the form:
```
<layout name>/init.lua++
e.g.: ~/.config/japokwm/layouts/foo/init.lua
```

# ACCESS FEATURES
We devide japokwm config files into 2 types. layout local and global config
files. They differ in the following ways:
- Global config are executed once per reload. Starting the window manager ++
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
	The objects opt and layout are either of layout local scope or global scope
	depending of the location. That means you can change the border width for a
	particular layout by writing:
```
opt.border_width = 10
```

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
boring and you may remember nothing. The best way to actually learn is play
around with various toggles you find in the rest of the documentation.

# HOW TO DEBUG
A file called init.err is generated in the directory your init.lua is in or if
that is not possible in the directory ~/.config/japokwm/init.err. Also you get a
notification when an error occurs. So setting up a notification daemon is
worthwhile.

# WHERE TO CONTINUE FROM HERE?
The most interesting features will be at *man 5 japokwm-options* and *man 5
japokwm-actions*. Also *man 5 japokwm-event_handler* may be interesting to you
to automate stuff

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

# LIST OF INTERESTING FEATURES THAT ARE HARD TO FIND ABOUT
- scratchpad
- monitor/container rules
- japokmsg see man 1 japokmsg
