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

; TODO add this in the future
; *-d, --debug*
; 	Enables full logging, including debug information.

; 	*-v, --version*
; 	Show the version number and quit.

; *-V, --verbose*
; 	Enables more verbose logging.

; *--get-socketpath*
; 	Gets the IPC socket path and prints it, then exits.

# DESCRIPTION

japokwm was created to create a tiling window manager where you can
create new layouts as easily as possible. We have to thank everyone that created
the wlroots library and the dwl project. Both of them made this project
possible.

You can run japokwm directly from a tty, or via a Wayland-compatible login manager.

# CONFIGURATION

japokwm searches for a the init.lua in the following locations, in this order:

. ~/.japokwm/config
. $XDG_CONFIG_HOME/sway/config (suggested location)
. ~/.i3/config
. $XDG_CONFIG_HOME/i3/config
. /etc/sway/config
. /etc/i3/config

If unset, $XDG_CONFIG_HOME defaults to *~/.config*.

An error is raised when no config file is found. The recommended default
configuration is usually installed to */etc/sway/config*; you are encouraged to
copy this to *~/.config/sway/config* and edit it from there.

For information on the config file, see *japokwm*(5).

# AUTHORS

Maintained by Jakob Schlanstedt

# SEE ALSO
