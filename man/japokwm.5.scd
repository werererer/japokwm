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
