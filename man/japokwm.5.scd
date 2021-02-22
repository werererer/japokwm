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
*focus_container(i)* - focus the container at i
	- i: integer - absolute position on the stack
	```
	action.focus_container(0) -- this focuses the master container
	```
*focus_on_hidden_stack(i)* - replace current container with a hidden container
	- i: integer - relative position on the hidden stack
	```
	action.focus_on_hidden_stack(0) -- show first hidden container
	```
*focus_on_stack(i)* - focuses the container at relative i
	- i: integer - relative position on the stack
*increase_nmaster()* - increase the number of master windows
# SEE ALSO

*japokwm*(1)
