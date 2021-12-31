japokwm-server(5)

# Type
	class Server
# Description
	A collection of functions to alter the state of the windowmanager.
# Methods
	Tag get_tag(int)
		Get the tag at position $1
	Tag get_focused_tag()
		Get the tag that is currently focused
	quit()
		Quit the windowmanager
# Variables
	ring_buffer default_layout_ring
		the default layout ring buffer
