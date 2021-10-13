opt.workspaces = {"0:1", "1:2", "2:3", "3:4", "4:5", "5:6", "6:7", "7:8"}
opt.sloppy_focus = true

-- focus follows mouse
opt.atomatic_workspace_naming = true;

local termcmd = "/usr/bin/alacritty"

local function on_start()
    -- execute programs or do what ever you want e.g.:
    -- action.exec("dbus-daemon --session --address=unix:path=$XDG_RUNTIME_DIR/bus")
    -- action.exec("xsetroot -cursor_name left_ptr")
end
-- executes function on_start when the 
event:add_listener("on_start", on_start)

opt.inner_gaps = 15
opt.border_color = Color.new(0.0, 0.0, 1.0, 1.0)
opt.focus_color = Color.new(1.0, 0.0, 0.0, 1.0)

local layouts = {"two_pane", "monocle", "tile"}

opt.create_layout_set("default", layouts)

layout.default_layout = layouts[1]

-- set it to 4 to use super instead
opt.mod = 1

local function exec_keycombo(i)
    if (info.is_keycombo("combo")) then
        action.tag_view(1 << i)
    else
        action.view(i)
    end
    action.start_keycombo("combo")
end

local function toggle_all_bars()
    for i = 1,info.get_workspace_count() do
        local ws_id = i-1
        local ws = server:get_workspace(ws_id)
        action.toggle_bars(ws, info.direction.all)
    end
end

local function get_entry_position(ws_id, is_focused)
    if (is_focused) then
        return 0
    else
        return 0
    end
end

local function get_focus_entry_position(ws_id, is_focused)
    if (is_focused) then
        return 0
    else
        return 0
    end
end

opt.entry_position_function = get_entry_position
opt.entry_focus_position_function = get_focus_entry_position

opt:bind_key("mod-S-p",       function() container.set_sticky(info.this_container_position(), 255) end)
opt:bind_key("mod-p",         function() action.exec("rofi -show run") end)
opt:bind_key("mod-e",         function() Workspace.view(info.get_next_empty_workspace(info.get_workspace(), info.direction.left)) end)
opt:bind_key("mod-period",    function() action.toggle_workspace() end)
opt:bind_key("mod-S-period",  function() action.toggle_layout() end)
opt:bind_key("mod-comma",     function() 
   local con = Container.get_focused()
   con.alpha = 0.4
   local workspace = server:get_workspace(0)
   server:get_focused_workspace():swap(server:get_workspace(3))
 end)
opt:bind_key("mod-S-Return",  function() action.exec(termcmd) end)
opt:bind_key("mod-a",         function() action.increase_nmaster() end)
opt:bind_key("mod-x",         function() action.decrease_nmaster() end)
opt:bind_key("mod-k",         function() action.focus_on_stack(-1) end)
opt:bind_key("mod-j",         function() action.focus_on_stack(1) end)
opt:bind_key("mod-S-j",       function() action.focus_on_hidden_stack(0) end)
opt:bind_key("mod-S-k",       function() action.focus_on_hidden_stack(-1) end)
opt:bind_key("mod-tab",       function() action.swap_on_hidden_stack(0) end)
opt:bind_key("mod-S-tab",     function() action.swap_on_hidden_stack(-1) end)
opt:bind_key("mod-S-c",       function() action.kill(Container.get_focused()) end)
opt:bind_key("mod-S-q",       function() server:quit() end)
opt:bind_key("mod-space",     function() action.load_next_layout_in_set("default") end)
opt:bind_key("mod-minus",     function() action.move_to_scratchpad(info.this_container_position()) end)
opt:bind_key("mod-S-minus",   function() action.show_scratchpad() end)
opt:bind_key("mod-S-space",   function() action.load_prev_layout_in_set("default") end)
opt:bind_key("mod-m",         function() action.focus_container(info.stack_position_to_position(0)) end)
opt:bind_key("mod-S-t",       function() action.load_layout_in_set("default", 2) end)
opt:bind_key("mod-w",         function() action.load_layout_in_set("default", 3) end)
opt:bind_key("mod-S-w",       function() action.load_layout_in_set("default", 4) end)
opt:bind_key("mod-b",         function() toggle_all_bars() end)
opt:bind_key("mod-S-h",       function() action.resize_main(-1/10) end)
opt:bind_key("mod-S-l",       function() action.resize_main(1/10) end)
opt:bind_key("mod-Return",    function() action.zoom() end)
opt:bind_key("mod-1",         function() exec_keycombo(0) end)
opt:bind_key("mod-2",         function() exec_keycombo(1) end)
opt:bind_key("mod-3",         function() exec_keycombo(2) end)
opt:bind_key("mod-4",         function() exec_keycombo(3) end)
opt:bind_key("mod-5",         function() exec_keycombo(4) end)
opt:bind_key("mod-6",         function() exec_keycombo(5) end)
opt:bind_key("mod-7",         function() exec_keycombo(6) end)
opt:bind_key("mod-8",         function() exec_keycombo(7) end)
opt:bind_key("mod-9",         function() exec_keycombo(8) end)
opt:bind_key("mod-0",         function() action.set_tags(1 << Container.get_focused().workspace.id) end)
opt:bind_key("mod-C-1",       function() action.tag_view(1 << 0) end)
opt:bind_key("mod-C-2",       function() action.tag_view(1 << 1) end)
opt:bind_key("mod-C-3",       function() action.tag_view(1 << 2) end)
opt:bind_key("mod-C-4",       function() action.tag_view(1 << 3) end)
opt:bind_key("mod-C-5",       function() action.tag_view(1 << 4) end)
opt:bind_key("mod-C-6",       function() action.tag_view(1 << 5) end)
opt:bind_key("mod-C-7",       function() action.tag_view(1 << 6) end)
opt:bind_key("mod-C-8",       function() action.tag_view(1 << 7) end)
opt:bind_key("mod-C-9",       function() action.tag_view(1 << 8) end)
opt:bind_key("mod-C-0",       function() action.set_tags(1 << Workspace.get_focused():get_id()) end)
opt:bind_key("mod-S-1",       function() action.move_container_to_workspace(0) end)
opt:bind_key("mod-S-2",       function() action.move_container_to_workspace(1) end)
opt:bind_key("mod-S-3",       function() action.move_container_to_workspace(2) end)
opt:bind_key("mod-S-4",       function() action.move_container_to_workspace(3) end)
opt:bind_key("mod-S-5",       function() action.move_container_to_workspace(4) end)
opt:bind_key("mod-S-6",       function() action.move_container_to_workspace(5) end)
opt:bind_key("mod-S-7",       function() action.move_container_to_workspace(6) end)
opt:bind_key("mod-S-8",       function() action.move_container_to_workspace(7) end)
opt:bind_key("mod-S-9",       function() action.move_container_to_workspace(8) end)
opt:bind_key("mod-C-S-0",     function() Container.get_focused():set_sticky_restricted(0) end)
opt:bind_key("mod-C-S-1",     function() Container.get_focused():toggle_add_sticky(1 << 0) end)
opt:bind_key("mod-C-S-2",     function() Container.get_focused():toggle_add_sticky(1 << 1) end)
opt:bind_key("mod-C-S-3",     function() Container.get_focused():toggle_add_sticky(1 << 2) end)
opt:bind_key("mod-C-S-4",     function() Container.get_focused():toggle_add_sticky(1 << 3) end)
opt:bind_key("mod-C-S-5",     function() Container.get_focused():toggle_add_sticky(1 << 4) end)
opt:bind_key("mod-C-S-6",     function() Container.get_focused():toggle_add_sticky(1 << 5) end)
opt:bind_key("mod-C-S-7",     function() Container.get_focused():toggle_add_sticky(1 << 6) end)
opt:bind_key("mod-C-S-8",     function() Container.get_focused():toggle_add_sticky(1 << 7) end)
opt:bind_key("mod-C-S-9",     function() Container.get_focused():set_sticky_restricted(255) end)
opt:bind_key("mod-s 1",       function() server:get_focused_workspace():swap(server:get_workspace(0)) end)
opt:bind_key("mod-s 2",       function() server:get_focused_workspace():swap(server:get_workspace(1)) end)
opt:bind_key("mod-s 3",       function() server:get_focused_workspace():swap(server:get_workspace(2)) end)
opt:bind_key("mod-s 4",       function() server:get_focused_workspace():swap(server:get_workspace(3)) end)
opt:bind_key("mod-s 5",       function() server:get_focused_workspace():swap(server:get_workspace(4)) end)
opt:bind_key("mod-s 6",       function() server:get_focused_workspace():swap(server:get_workspace(5)) end)
opt:bind_key("mod-s 7",       function() server:get_focused_workspace():swap(server:get_workspace(6)) end)
opt:bind_key("mod-s 8",       function() server:get_focused_workspace():swap(server:get_workspace(7)) end)
opt:bind_key("mod-r",         function() opt.reload() end)
opt:bind_key("mod-t",         function() action.set_floating(false)    end)
opt:bind_key("mod-M1",  function() action.move_resize(info.cursor.mode.move) end)
opt:bind_key("mod-M2",  function() action.move_resize(info.cursor.mode.resize) end)
opt:bind_key("M1", function() action.focus_container(info.get_container_under_cursor()) end)
