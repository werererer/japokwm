config.create_workspaces({"0:1", "1:2", "2:3", "3:4", "4:5", "5:6", "6:7", "7:8"})

-- focus follows mouse
config.set_sloppy_focus(true)

config.set_automatic_workspace_naming(true)

local termcmd = "/usr/bin/alacritty"

local function on_start()
    -- execute programs or do what ever you want e.g.:
    -- action.exec("dbus-daemon --session --address=unix:path=$XDG_RUNTIME_DIR/bus")
    -- action.exec("xsetroot -cursor_name left_ptr")
end
-- executes function on_start when the 
event.add_listener("on_start", on_start)

config.set_inner_gaps(0)

local layouts = {"two_pane", "monocle", "tile"}

config.create_layout_set("default", layouts)

config.set_default_layout(layouts[1])

-- set it to 4 to use super instead
config.set_mod(1)

local function exec_keycombo(i)
    if (info.is_keycombo("combo")) then
        action.tag_view(1 << i)
    else
        action.view(i)
    end
    action.start_keycombo("combo")
end

config.bind_key("mod-S-p",       function() container.set_sticky(info.this_container_position(), 255) end)
config.bind_key("mod-p",         function() action.exec("rofi -show run") end)
config.bind_key("mod-e",         function() action.view(info.get_next_empty_workspace(info.get_workspace(), info.direction.left)) end)
config.bind_key("mod-period",    function() action.toggle_workspace() end)
config.bind_key("mod-S-period",  function() action.toggle_layout() end)
config.bind_key("mod-S-Return",  function() action.exec(termcmd) end)
config.bind_key("mod-a",         function() action.increase_nmaster() end)
config.bind_key("mod-x",         function() action.decrease_nmaster() end)
config.bind_key("mod-k",         function() action.focus_on_stack(-1) end)
config.bind_key("mod-j",         function() action.focus_on_stack(1) end)
config.bind_key("mod-S-j",       function() action.focus_on_hidden_stack(0) end)
config.bind_key("mod-S-k",       function() action.focus_on_hidden_stack(-1) end)
config.bind_key("mod-S-c",       function() action.kill(info.this_container_position()) end)
config.bind_key("mod-S-q",       function() action.quit() end)
config.bind_key("mod-space",     function() action.load_next_layout_in_set("default") end)
config.bind_key("mod-minus",     function() action.move_to_scratchpad(info.this_container_position()) end)
config.bind_key("mod-S-minus",   function() action.show_scratchpad() end)
config.bind_key("mod-S-space",   function() action.load_prev_layout_in_set("default") end)
config.bind_key("mod-m",         function() action.focus_container(info.stack_position_to_position(0)) end)
config.bind_key("mod-S-t",       function() action.load_layout_in_set("default", 2) end)
config.bind_key("mod-w",         function() action.load_layout_in_set("default", 3) end)
config.bind_key("mod-S-w",       function() action.load_layout_in_set("default", 4) end)
config.bind_key("mod-b",         function() action.toggle_bars() end)
config.bind_key("mod-S-h",       function() action.resize_main(-1/10) end)
config.bind_key("mod-S-l",       function() action.resize_main(1/10) end)
config.bind_key("mod-Return",    function() action.zoom() end)
config.bind_key("mod-1",         function() exec_keycombo(0) end)
config.bind_key("mod-2",         function() exec_keycombo(1) end)
config.bind_key("mod-3",         function() exec_keycombo(2) end)
config.bind_key("mod-4",         function() exec_keycombo(3) end)
config.bind_key("mod-5",         function() exec_keycombo(4) end)
config.bind_key("mod-6",         function() exec_keycombo(5) end)
config.bind_key("mod-7",         function() exec_keycombo(6) end)
config.bind_key("mod-8",         function() exec_keycombo(7) end)
config.bind_key("mod-9",         function() exec_keycombo(8) end)
config.bind_key("mod-C-1",       function() action.tag_view(1 << 0) end)
config.bind_key("mod-C-2",       function() action.tag_view(1 << 1) end)
config.bind_key("mod-C-3",       function() action.tag_view(1 << 2) end)
config.bind_key("mod-C-4",       function() action.tag_view(1 << 3) end)
config.bind_key("mod-C-5",       function() action.tag_view(1 << 4) end)
config.bind_key("mod-C-6",       function() action.tag_view(1 << 5) end)
config.bind_key("mod-C-7",       function() action.tag_view(1 << 6) end)
config.bind_key("mod-C-8",       function() action.tag_view(1 << 7) end)
config.bind_key("mod-C-9",       function() action.tag_view(1 << 8) end)
config.bind_key("mod-S-1",       function() action.move_container_to_workspace(0) end)
config.bind_key("mod-S-2",       function() action.move_container_to_workspace(1) end)
config.bind_key("mod-S-3",       function() action.move_container_to_workspace(2) end)
config.bind_key("mod-S-4",       function() action.move_container_to_workspace(3) end)
config.bind_key("mod-S-5",       function() action.move_container_to_workspace(4) end)
config.bind_key("mod-S-6",       function() action.move_container_to_workspace(5) end)
config.bind_key("mod-S-7",       function() action.move_container_to_workspace(6) end)
config.bind_key("mod-S-8",       function() action.move_container_to_workspace(7) end)
config.bind_key("mod-S-9",       function() action.move_container_to_workspace(8) end)
config.bind_key("mod-C-S-0",     function() container.set_sticky_restricted(info.this_container_position(), 0) end)
config.bind_key("mod-C-S-1",     function() container.toggle_add_sticky(info.this_container_position(), 1 << 0) end)
config.bind_key("mod-C-S-2",     function() container.toggle_add_sticky(info.this_container_position(), 1 << 1) end)
config.bind_key("mod-C-S-3",     function() container.toggle_add_sticky(info.this_container_position(), 1 << 2) end)
config.bind_key("mod-C-S-4",     function() container.toggle_add_sticky(info.this_container_position(), 1 << 3) end)
config.bind_key("mod-C-S-5",     function() container.toggle_add_sticky(info.this_container_position(), 1 << 4) end)
config.bind_key("mod-C-S-6",     function() container.toggle_add_sticky(info.this_container_position(), 1 << 5) end)
config.bind_key("mod-C-S-7",     function() container.toggle_add_sticky(info.this_container_position(), 1 << 6) end)
config.bind_key("mod-C-S-8",     function() container.toggle_add_sticky(info.this_container_position(), 1 << 7) end)
config.bind_key("mod-C-S-9",     function() container.set_sticky_restricted(info.this_container_position(), 255) end)
config.bind_key("mod-s 1",       function() action.swap_workspace(info.get_workspace(), 0) end)
config.bind_key("mod-s 2",       function() action.swap_workspace(info.get_workspace(), 1) end)
config.bind_key("mod-s 3",       function() action.swap_workspace(info.get_workspace(), 2) end)
config.bind_key("mod-s 4",       function() action.swap_workspace(info.get_workspace(), 3) end)
config.bind_key("mod-s 5",       function() action.swap_workspace(info.get_workspace(), 4) end)
config.bind_key("mod-s 6",       function() action.swap_workspace(info.get_workspace(), 5) end)
config.bind_key("mod-s 7",       function() action.swap_workspace(info.get_workspace(), 6) end)
config.bind_key("mod-s 8",       function() action.swap_workspace(info.get_workspace(), 7) end)
config.bind_key("mod-r",         function() config.reload() end)
config.bind_key("mod-t",         function() action.set_floating(false)    end)
config.bind_key("mod-M1",  function() action.move_resize(info.cursor.mode.move) end)
config.bind_key("mod-M2",  function() action.move_resize(info.cursor.mode.resize) end)
config.bind_key("M1", function() action.focus_container(info.get_container_under_cursor()) end)
