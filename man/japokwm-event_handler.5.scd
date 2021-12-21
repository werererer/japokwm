japokwm-event_handler(5)

# Type
	class Event
# Description
	Event handler for the event system.
# Methods
	void add_listener(string, function)
		Adds a listener function $2 to the event system. $1 is the event
		type.
# EventTypes
	on_start
		Called when the window manager is started.
	on_focus
		Called when a window is focused.++
args:
			container:
				The container that was unfocused.
	on_unfocus
		Called when a window loses focus.++
args:
			container:
				The container that was unfocused.
	on_update
		Called when the layout is rearranged.++
args:
			layout:
				The layout object of the updated layout.

	on_create_container
		Called when a new container is created.++
args:
			container:
				The container that was created.
