japokwm(1)

# NAME

japokwm - tiling made easy

# SYNOPSIS

*japokwm* [options...] [command]

# OPTIONS

*-h, --help*
	Show help message and quit.

*-c, --config* <config>
	Specifies a config file.

*-p, --path* <path>
	Specifies the directory where the config files are located in (Note: -c
	option overwrites the init.lua found in the path)

*-v, --version*
	Show the version number and quit.

# DESCRIPTION

japokwm is a tiling window manager where you can create new layouts as easily as
possible. We have to thank everyone that created the wlroots library and the dwl
project. Both of them made this project possible.

You can run japokwm directly from a tty, or via a Wayland-compatible login manager.

# CONFIGURATION

japokwm searches for the init.lua file in the following locations, in this order:

. $XDG_CONFIG_HOME/japokwm
. ~/.config/japokwm (suggested location)
. /etc/japokwm/

The file ~/.config/japokwm/init.err will get populated when an error is raised.
The recommended default configuration is usually installed to
*/etc/japokwm/init.lua*; you are encouraged to copy this and all other files
from there to the directory *~/.config/japokwm/* and edit them from there.

For information on the config file, see *japokwm*(5).

# Layouts
Note: examples can usually be found at /etc/japokwm/layouts

All layouts are searched in the following locations:

. $XDG_CONFIG_HOME/japokwm/layouts
. ~/.config/japokwm/layouts (suggested location)
. /etc/japokwm/layouts

If you want to create a layout named foo you need to create a directory called
foo in either of these locations. Then you need to create the file init.lua in 
that directory. That means:
```
mkdir -p $HOME/.config/japokwm/layouts/foo
cd $HOME/.config/japokwm/layouts/foo
touch init.lua
```
Now you need to enter the basic information for a layout:
```
local layout_data = {
	{
		{0, 0, 1, 1},
	},
	{
		{0.0, 0.0, 0.5, 1.0},
		{0.5, 0.0, 0.5, 1.0},
	},
}

layout.set("foo", layout_data)
```
Now add the new layout to the layouts array in ~/.config/japokwm/init.lua: ++
Replace:
```
local layouts = {"two_pane", "monocle", "tile"}
```
with:
```
local layouts = {"two_pane", "monocle", "tile", "foo"}
```

that was it.

for more info checkout `man 5 japokwm`

# AUTHORS

Maintained by Jakob Schlanstedt

# SEE ALSO
