japokwm(5)

# DESCRIPTION
A japokwm configuration file is a lua file that is executed by japokwm. You can
use built-in functions with it to change the behavior of your window manager and
change various settings. An example config is likely present in
/etc/japokwm/init.lua for you to check out.

# Terminology
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
the container and the content you see in it
