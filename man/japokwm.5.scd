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
*arrange* - Arrange containers to the given information
	```
	action.arrange()
	```
*decrease_nmaster* - decrease the number of master windows
	```
	action.decrease_nmaster()
	```
*focus_container* - focus a container that is at the given position
	```
	action.focus_container(3)
	```
*focus_on_hidden_stack* - replace current container with the
*increase_nmaster* - increase the number of master windows
	```
	action.increase_nmaster()
	```
*focus_container* - focuses
# SEE ALSO

*japokwm*(1)
