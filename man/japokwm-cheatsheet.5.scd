japokwm-cheatsheet(5)

# Description
These are the default keybindings japokwm will assign loosely ordered by
importance.

# Keybindings
mod-S-q
	quit the window managerx
	server:quit()

mod-r
	opt.reload()

mod-S-Return
	Action.exec("/usr/bin/alacritty")

mod-p
	Action.exec("rofi -show run")

mod-S-c
	if Container.focused then
		Container.focused:kill()
	end

mod-j
	Action.focus_on_stack(1)

mod-k
	Action.focus_on_stack(-1)

mod-Return
	Action.zoom()

mod-t
	Container.focused.property.floating = false

mod-M1
	Action.move_resize(Cursor_mode.move)

mod-M2
	Action.move_resize(Cursor_mode.resize)

M1
	local con = Info.get_container_under_cursor()
	if con then
		con:focus()
	end

mod-S-h
	Action.resize_main(-1/10)

mod-S-l
	Action.resize_main(1/10)

mod-1
	Action.view_or_tag(1)

mod-2
	Action.view_or_tag(2)

mod-3
	Action.view_or_tag(3)

mod-4
	Action.view_or_tag(4)

mod-5
	Action.view_or_tag(5)

mod-6
	Action.view_or_tag(6)

mod-7
	Action.view_or_tag(7)

mod-8
	Action.view_or_tag(8)

mod-9
	Action.view_or_tag(9)

mod-S-j
	Action.focus_on_hidden_stack(0)

mod-S-k
	Action.focus_on_hidden_stack(-1)

mod-b
	Tag.focused:toggle_bars()

mod-S-1
	if Container.focused then
		Container.focused.tag = Tag.get(1)
	end

mod-S-2
	if Container.focused then
		Container.focused.tag = Tag.get(2)
	end

mod-S-3
	if Container.focused then
		Container.focused.tag = Tag.get(3)
	end

mod-S-4
	if Container.focused then
		Container.focused.tag = Tag.get(4)
	end

mod-S-5
	if Container.focused then
		Container.focused.tag = Tag.get(5)
	end

mod-S-6
	if Container.focused then
		Container.focused.tag = Tag.get(6)
	end

mod-S-7
	if Container.focused then
		Container.focused.tag = Tag.get(7)
	end

mod-S-8
	if Container.focused then
		Container.focused.tag = Tag.get(8)
	end

mod-S-9
	if Container.focused then
		Container.focused.tag = Tag.get(9)
	end

mod-C-1
	Tag.focused.tags:\_xor(1 << 0)

mod-C-2
	Tag.focused.tags:\_xor(1 << 1)

mod-C-3
	Tag.focused.tags:\_xor(1 << 2)

mod-C-4
	Tag.focused.tags:\_xor(1 << 3)

mod-C-5
	Tag.focused.tags:\_xor(1 << 4)

mod-C-6
	Tag.focused.tags:\_xor(1 << 5)

mod-C-7
	Tag.focused.tags:\_xor(1 << 6)

mod-C-8
	Tag.focused.tags:\_xor(1 << 7)

mod-C-9
	Tag.focused.tags:\_xor(1 << 8)

mod-C-S-1
	Container.focused.tags:\_xor(1 << 0)

mod-C-S-2
	Container.focused.tags:\_xor(1 << 1)

mod-C-S-3
	Container.focused.tags:\_xor(1 << 2)

mod-C-S-4
	Container.focused.tags:\_xor(1 << 3)

mod-C-S-5
	Container.focused.tags:\_xor(1 << 4)

mod-C-S-6
	Container.focused.tags:\_xor(1 << 5)

mod-C-S-7
	Container.focused.tags:\_xor(1 << 6)

mod-C-S-8
	Container.focused.tags:\_xor(1 << 7)

mod-s 1
	Tag.focused:swap_smart(Tag.get(1))

mod-s 2
	Tag.focused:swap_smart(Tag.get(2))

mod-s 3
	Tag.focused:swap_smart(Tag.get(3))

mod-s 4
	Tag.focused:swap_smart(Tag.get(4))

mod-s 5
	Tag.focused:swap_smart(Tag.get(5))

mod-s 6
	Tag.focused:swap_smart(Tag.get(6))

mod-s 7
	Tag.focused:swap_smart(Tag.get(7))

mod-s 8
	Tag.focused:swap_smart(Tag.get(8))

mod-a
	Layout.focused:increase_n_master()

mod-x
	Layout.focused:decrease_n_master()

mod-m
	Tag.focused.stack[1]:focus()

mod-e
	Action.view(tag.focused:get_next_empty(Direction.right))

mod-S-space
	Layout.load(server.default_layout_ring:prev())

mod-space
	Layout.load(server.default_layout_ring:next())

mod-minus
	if Container.focused then
		Action.move_to_scratchpad(Container.focused)
	end

mod-S-minus
	Action.show_scratchpad()

mod-tab
	Action.swap_on_hidden_stack(0)

mod-S-tab
	Action.swap_on_hidden_stack(-1)

mod-period
	Action.toggle_tag()

mod-S-period
	Action.toggle_tags()

mod-0
	Tag.focused.tags = 1 << Tag.focused:get_id()

mod-S-0
	local con = Container.focused
	if con then
		tag.focused.tags = 1 << con.tag:get_id()
	end

mod-C-S-0
	local con = Container.focused
	if con then
		con.sticky_restricted = 0
	end

mod-C-S-9
	local con = Container.focused
	if con then
		con.sticky_restricted = 255
	end
