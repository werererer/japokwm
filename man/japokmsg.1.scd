japokmsg(1)

# NAME

japokmsg - Send messages to a running instance of sway over the IPC socket.

# SYNOPSIS

japokmsg [options...] [command]

# OPTIONS

*-h, --help*
	Show help message and quit.

*-p, --pretty*
	Use pretty output even when not using a tty.

*-q, --quiet*
	Sends the IPC message but does not print the response from japokwm.

*-r, --raw*
	Use raw output even if using a tty.

*-s, --socket* <path>
	Use the specified socket path. Otherwise, japokmsg will ask japokwm where
	the socket is (which is the value of $JAPOKWMSOCK).

*-v, --version*
	Print the version (of japokmsg) and quit.

# Command
	The command is just lua code that will be executed by japokwm. The scope is
	Layout local.

*japokwm*(5)
