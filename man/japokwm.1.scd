japokwm(1)

# NAME

japokwm - tiling made easy

# SYNOPSIS

*sway* [options...] [command]

# OPTIONS

*-h, --help*
	Show help message and quit.

*-c, --config* <config>
	Specifies a config file.

*-d, --debug*
	Enables full logging, including debug information.

*-v, --version*
	Show the version number and quit.

*-V, --verbose*
	Enables more verbose logging.

*--get-socketpath*
	Gets the IPC socket path and prints it, then exits.

# DESCRIPTION

japokwm was created to finally create a tiling windowmanager where you can
create new layouts as easily as possible. We have to thank everyone that created
the wlroots library and the dwl project. Both of them made this project
possible.

You can run japokwm directly from a tty, or via a Wayland-compatible login manager.

# CONFIGURATION

japokwm searches for a the init.lua in the following locations, in this order:

# TODO continue from here

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

For information on the config file format, see *sway*(5).

# IPC COMMANDS

Though *swaymsg*(1) is generally preferred, you may run *sway* _command_ to
send _command_ to the running instance of sway. You can also issue commands
with *i3-msg*(1) or even with *i3*(1).

# ENVIRONMENT

The following environment variables have an effect on sway:

_SWAYSOCK_
	Specifies the path to the sway IPC socket.

_XKB\_DEFAULT\_RULES_, _XKB\_DEFAULT\_MODEL_, _XKB\_DEFAULT\_LAYOUT_,
_XKB\_DEFAULT\_VARIANT_, _XKB\_DEFAULT\_OPTIONS_
	Configures the xkb keyboard settings. See *xkeyboard-config*(7). The
	preferred way to configure the keyboard is via the configuration file, see
	*sway-input*(5).

The following environment variables are set by sway:

_DISPLAY_
	If compiled with Xwayland support and Xwayland is not disabled by the
	config, this will be set to the name of the X display used for Xwayland.

_I3SOCK_
	For compatibility with i3, specifies the path to the sway IPC socket.

_SWAYSOCK_
	Specifies the path to the sway IPC socket.

_WAYLAND_DISPLAY_
	Specifies the name of the Wayland display that sway is running on.

_XCURSOR_SIZE_
	Specifies the configured cursor size.

_XCURSOR_THEME_
	Specifies the configured cursor theme.

# AUTHORS

Maintained by Drew DeVault <sir@cmpwn.com>, who is assisted by other open
source contributors. For more information about sway development, see
<https://github.com/swaywm/sway>.

# SEE ALSO

*sway*(5) *swaymsg*(1) *sway-input*(5) *sway-output*(5) *sway-bar*(5)
*sway-ipc*(7)
