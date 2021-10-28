opt.sloppy_focus = true
opt.automatic_workspace_naming = true
opt.border_width = 2
opt.float_border_width = 2
opt.root_color = Color.new(0.3, 0.3, 0.3, 1.0)
opt.focus_color = Color.new(1.0, 0.0, 0.0, 1.0)
opt.border_color = Color.new(0.0, 0.0, 1.0, 1.0)
opt:set_repeat_rate(25)
opt:set_repeat_delay(600)

opt.workspaces = {"0:1", "1:2", "2:3", "3:4", "4:5", "5:6", "6:7", "7:8"}

local function on_start()
    action.exec("dbus-daemon --session --address=unix:path=$XDG_RUNTIME_DIR/bus")
    action.exec("xsetroot -cursor_name left_ptr")
    action.exec("kdeconnect-indicator")
    action.exec("swaybg -i ~/wallpaper/00-breakdown")
    action.exec("waybar")
    action.exec("fcitx5")
end

layout:set_master_layout_data(
{{{0, 0, 1, 1}}, {{0, 0, 0.5, 1}, {0.5, 0, 0.5, 1}}}
)

event:add_listener("on_start", on_start)

local layouts = {"three_columns", "two_pane",}

opt:add_rule({ class = "", callback = function(con) con.ratio = 0 end})

-- config.add_mon_rule({callback = function() monitor.set_scale(0.6) end})
-- config.add_mon_rule({callback = function() monitor.set_transform(info.monitor.transform.rotate_90) end})

opt:create_layout_set("default", layouts)

layout.default_layout = layouts[1]
local function toggle_fullscreen()
    local active_layout = info.get_active_layout()
    local is_fullscreen = active_layout == "monocle"
    if is_fullscreen then
        local last_layout = info.get_previous_layout()
        action.load_layout(last_layout)
    else
        action.load_layout("monocle")

    end
    is_fullscreen = not is_fullscreen
end

opt.mod = 4

local keycombo = false

local function exec_keycombo(i)
    if (info.is_keycombo("combo")) then
        Workspace.get_focused().tags:_xor(1 << (i-1))
    else
        action.view(Workspace.get(i))
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

local termcmd = "/usr/bin/alacritty"
opt:bind_key("XF86AudioRaiseVolume", function() action.exec("pactl set-sink-volume @DEFAULT_SINK@ +5%") end)
opt:bind_key("XF86AudioLowerVolume", function() action.exec("pactl set-sink-volume @DEFAULT_SINK@ -5%") end)
opt:bind_key("XF86AudioMute", function() action.exec("amixer -D pulse set Master toggle") end)
opt:bind_key("mod-p",         function() action.exec("rofi -show run") end)
opt:bind_key("mod-e",         function() action.view(Workspace.get_next_empty(Workspace.get_focused(), info.direction.left):get_id()) end)
opt:bind_key("mod-S-e",       function() action.move_container_to_workspace(info.get_next_empty_workspace(info.get_workspace(), info.direction.left)) end)
opt:bind_key("mod-period",    function() action.toggle_workspace() end)
opt:bind_key("mod-comma",     function() action.toggle_tags() end)
opt:bind_key("mod-plus", function() action.exec("wob 30")end)
-- opt:bind_key("Alt_L-Tab", function()
--     action.async_execute(function()
--         local ws = Workspace.get_focused()
--         local focus_stack = ws.focus_stack
--
--         local str = ""
--         for i,con in ipairs(focus_stack) do
--             str = str .. con.app_id .. "\n"
--         end
--         local dmenu = "rofi -dmenu"
--         local echo = 'echo "' .. str .. '"'
--         local pipe = "|"
--         local cmd = echo .. pipe .. dmenu
--         print(cmd)
--         local handle = io.popen(cmd)
--         local result = handle:read("*a")
--         handle:close()
--
--          -- find container
--         local s_con = nil
--         for i,con in ipairs(focus_stack) do
--             local res = result:gsub("%s+", "")
--             local app_id = con.app_id:gsub("%s+", "")
--             if res == app_id then
--                 s_con = con
--                 break;
--             end
--         end
--
--         if s_con then
--             local ws = Workspace.get_focused()
--             local ws_id = s_con.workspace:get_id()
--             ws.tags:_or(1 << ws_id)
--             ws.stack:repush(ws.stack:find(s_con), 1)
--         end
--     end)
-- end)
-- opt:bind_key("Alt_L-Tab", function()
--     action.async_execute(function()
--         local ws = Workspace.get_focused()
--         print(type(Workspace.get_focused()))
--         local focus_stack = ws.focus_stack
--         focus_stack[1], focus_stack[2] = focus_stack[2], focus_stack[1]
--         local str = ""
--         for i,con in ipairs(focus_stack) do
--             str = str .. con.app_id .. "\n"
--         end
--         local dmenu = "rofi -dmenu"
--         local echo = 'echo "' .. str .. '"'
--         local pipe = "|"
--         local cmd = echo .. pipe .. dmenu
--         print(cmd)
--         local handle = io.popen(cmd)
--         local result = handle:read("*a")
--         handle:close()
--
--         local s_con = nil
--         for i,con in ipairs(focus_stack) do
--             local res = result:gsub("%s+", "")
--             local app_id = con.app_id:gsub("%s+", "")
--             print("result: ", res)
--             print("app_id: ", app_id)
--             print("equ: ", res == app_id)
--             if res == app_id then
--                 s_con = con
--                 break;
--             end
--         end
--
--         if s_con then
--             local ws = s_con.workspace
--             action.view(ws:get_id())
--         end
--     end)
-- end)
opt:bind_key("mod-S-period",  function() action.toggle_layout() end)
opt:bind_key("mod-S-Return",  function() action.exec(termcmd) end)
opt:bind_key("mod-a",         function() action.increase_nmaster() end)
opt:bind_key("mod-minus",     function() action.move_to_scratchpad(info.this_container_position()) end)
opt:bind_key("mod-S-minus",   function() action.show_scratchpad() end)
opt:bind_key("mod-x",         function() action.decrease_nmaster() end)
opt:bind_key("mod-k",         function() action.focus_on_stack(-1) end)
opt:bind_key("mod-j",         function() action.focus_on_stack(1) end)
opt:bind_key("mod-Tab",       function() action.focus_on_hidden_stack(0) end)
opt:bind_key("mod-S-Tab",     function() action.focus_on_hidden_stack(-1) end)
-- config.bind_key("mod-comma",     function() action.swap_on_hidden_stack(-1) end)
-- config.bind_key("mod-S-comma",   function() action.swap_on_hidden_stack(0) end)
opt:bind_key("mod-S-j",       function() action.repush(info.get_n_tiled()-1, 1) end)
opt:bind_key("mod-S-k",       function() action.repush(1, info.get_n_tiled()-1) end)
opt:bind_key("mod-space",     function() action.load_next_layout_in_set("default") end)
opt:bind_key("mod-S-space",   function() action.load_prev_layout_in_set("default") end)
opt:bind_key("mod-m",         function() action.focus_container(info.stack_position_to_position(0)) end)
opt:bind_key("mod-S-t",         function() action.load_layout_in_set("default", 1) end)
opt:bind_key("mod-w",         function() action.load_layout_in_set("default", 2) end)
opt:bind_key("mod-S-m",       function() toggle_fullscreen() end)
opt:bind_key("mod-b",         function() toggle_all_bars() end)
opt:bind_key("mod-S-h",         function() action.resize_main(-1/10) end)
opt:bind_key("mod-S-l",         function() action.resize_main(1/10) end)
opt:bind_key("mod-S-s",       function() action.exec("grim -g \"$(slurp -d)\" - | wl-copy") end)
opt:bind_key("mod-Return",    function() action.zoom() end)
opt:bind_key("mod-1",         function() exec_keycombo(1) end)
opt:bind_key("mod-2",         function() exec_keycombo(2) end)
opt:bind_key("mod-3",         function() exec_keycombo(3) end)
opt:bind_key("mod-4",         function() exec_keycombo(4) end)
opt:bind_key("mod-5",         function() exec_keycombo(5) end)
opt:bind_key("mod-6",         function() exec_keycombo(6) end)
opt:bind_key("mod-7",         function() exec_keycombo(7) end)
opt:bind_key("mod-8",         function() exec_keycombo(8) end)
opt:bind_key("mod-9",         function() exec_keycombo(9) end)
opt:bind_key("mod-0",         function()
    Workspace.get_focused().tags = 1 << Workspace.get_focused():get_id()
end)
opt:bind_key("mod-C-1",       function() Workspace.get_focused().tags:_xor(1 << 0) end)
opt:bind_key("mod-C-2",       function() Workspace.get_focused().tags:_xor(1 << 1) end)
opt:bind_key("mod-C-3",       function() Workspace.get_focused().tags:_xor(1 << 2) end)
opt:bind_key("mod-C-4",       function() Workspace.get_focused().tags:_xor(1 << 3) end)
opt:bind_key("mod-C-5",       function() Workspace.get_focused().tags:_xor(1 << 4) end)
opt:bind_key("mod-C-6",       function() Workspace.get_focused().tags:_xor(1 << 5) end)
opt:bind_key("mod-C-7",       function() Workspace.get_focused().tags:_xor(1 << 6) end)
opt:bind_key("mod-C-8",       function() Workspace.get_focused().tags:_xor(1 << 7) end)
opt:bind_key("mod-C-9",       function() Workspace.get_focused().tags:_xor(1 << 8) end)
opt:bind_key("mod-S-0",       function()
    local focused_con = Container.get_focused()
    if focused_con then
        local ws = focused_con.workspace
        action.view(ws)
        ws.tags = 1 << ws:get_id()
    end
 end)
opt:bind_key("mod-S-1",       function() Container.get_focused().workspace = Workspace.get(1) end)
opt:bind_key("mod-S-2",       function() Container.get_focused().workspace = Workspace.get(2) end)
opt:bind_key("mod-S-3",       function() Container.get_focused().workspace = Workspace.get(3) end)
opt:bind_key("mod-S-4",       function() Container.get_focused().workspace = Workspace.get(4) end)
opt:bind_key("mod-S-5",       function() Container.get_focused().workspace = Workspace.get(5) end)
opt:bind_key("mod-S-6",       function() Container.get_focused().workspace = Workspace.get(6) end)
opt:bind_key("mod-S-7",       function() Container.get_focused().workspace = Workspace.get(7) end)
opt:bind_key("mod-S-8",       function() Container.get_focused().workspace = Workspace.get(8) end)
opt:bind_key("mod-S-9",       function() Container.get_focused().workspace = Workspace.get(9) end)
opt:bind_key("mod-C-S-0",     function() container.set_sticky_restricted(info.this_container_position(), 0) end)
opt:bind_key("mod-C-S-1",     function() container.toggle_add_sticky(info.this_container_position(), 1 << 0) end)
opt:bind_key("mod-C-S-2",     function() container.toggle_add_sticky(info.this_container_position(), 1 << 1) end)
opt:bind_key("mod-C-S-3",     function() container.toggle_add_sticky(info.this_container_position(), 1 << 2) end)
opt:bind_key("mod-C-S-4",     function() container.toggle_add_sticky(info.this_container_position(), 1 << 3) end)
opt:bind_key("mod-C-S-5",     function() container.toggle_add_sticky(info.this_container_position(), 1 << 4) end)
opt:bind_key("mod-C-S-6",     function() container.toggle_add_sticky(info.this_container_position(), 1 << 5) end)
opt:bind_key("mod-C-S-7",     function() container.toggle_add_sticky(info.this_container_position(), 1 << 6) end)
opt:bind_key("mod-C-S-8",     function() container.toggle_add_sticky(info.this_container_position(), 1 << 7) end)
opt:bind_key("mod-C-S-9",     function() container.set_sticky_restricted(info.this_container_position(), 255) end)
opt:bind_key("mod-s 1",       function() Workspace.get_focused():swap(Workspace.get(1)) end)
opt:bind_key("mod-s 2",       function() Workspace.get_focused():swap(Workspace.get(2)) end)
opt:bind_key("mod-s 3",       function() Workspace.get_focused():swap(Workspace.get(3)) end)
opt:bind_key("mod-s 4",       function() Workspace.get_focused():swap(Workspace.get(4)) end)
opt:bind_key("mod-s 5",       function() Workspace.get_focused():swap(Workspace.get(5)) end)
opt:bind_key("mod-s 6",       function() Workspace.get_focused():swap(Workspace.get(6)) end)
opt:bind_key("mod-s 7",       function() Workspace.get_focused():swap(Workspace.get(7)) end)
opt:bind_key("mod-s 8",       function() Workspace.get_focused():swap(Workspace.get(8)) end)
opt:bind_key("mod-r",         function() opt.reload() end)
opt:bind_key("mod-t",         function() action.set_floating(false)    end)
opt:bind_key("M1", function() action.focus_container(info.get_container_under_cursor()) end)
opt:bind_key("mod-M1",  function() action.move_resize(info.cursor.mode.move) end)
opt:bind_key("mod-M2",  function() action.move_resize(info.cursor.mode.resize) end)
opt:bind_key("mod-C-Return",  function() action.resize_to_cursor() end)
print("reload")
