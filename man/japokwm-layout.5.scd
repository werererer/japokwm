japokwm-layout(5)

# NAME
	Layout
# Description
	Modify the layout of the windows in the current tag.
# Functions
	void load(string)
		Load the layout at the directory with the name $1.
	void toggle()
		Toggle the layout with the previous one.
# Methods
	void decrease_n_master()
		Decrease the number of master windows.
	void increase_n_master()
		Increase the number of master windows.
	void set(layout_data)
		Sets the layout data at $1.
	void set_linked_layouts(layout[])
		$1 is the array of other layouts that also resize when the
		current layout is resized.
	void set_master_layout_data(layout_data)
		$1 is the layout data of the master area.
	void set_resize_data(resize_data)
		set the resize_data for the layout.
	; TODO: this is too unspecific.
	void set_resize_function(function)
		set the resize function for the layout.
# Static Variables
	layout focused
		The layout of the currently focused tag.
# Variables
	direction direction
		The direction the layout is going to resize.
	layout_data layout_data
		The current state of the layout data of the layout.
	int n_area
		The number of areas in the layout.
	int n_master
		The number of master windows in the layout.
	string name
		The name of the layout.
	layout_data o_layout_data
		The layout data of the layout.
	resize_data resize_data
		The resize data of the layout.
