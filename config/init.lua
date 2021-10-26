opt.workspaces = {"0:1", "1:2", "2:3", "3:4", "4:5", "5:6", "6:7", "7:8"}
opt.sloppy_focus = true

-- focus follows mouse
opt.automatic_workspace_naming = true;

local termcmd = "/usr/bin/alacritty"

local function on_start()
    -- execute programs or do what ever you want e.g.:
    -- action.exec("dbus-daemon --session --address=unix:path=$XDG_RUNTIME_DIR/bus")
    -- action.exec("xsetroot -cursor_name left_ptr")
end
-- executes function on_start when the 
event:add_listener("on_start", on_start)

local function on_focus(con)
    if con then
        con.alpha = 1.0
    end
end
local function on_unfocus(con)
    if con then
        con.alpha = 0.8
    end
end
event:add_listener("on_focus", on_focus)
event:add_listener("on_unfocus", on_unfocus)

opt.inner_gaps = 15
opt.border_color = Color.new(0.0, 0.0, 1.0, 1.0)
opt.focus_color = Color.new(1.0, 0.0, 0.0, 1.0)

-- opt:add_rule({ class = "termite", callback = function(con) con.set_sticky(n, true) end})
-- opt:add_rule({ class = "Alacritty", callback = function(con) con.ratio = 1 end})

local layouts = {"two_pane", "tile"}

opt.create_layout_set("default", layouts)

layout.default_layout = "two_pane"

-- set it to 4 to use super instead
opt.mod = 1

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
        action.toggle_bars(ws, Direction.all)
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
opt:bind_key("mod-e",         function() action.view(Workspace.get_focused():get_next_empty(Direction.left):get_id()) end)
opt:bind_key("mod-period",    function() action.toggle_workspace() end)
opt:bind_key("mod-S-period",  function() action.toggle_tags() end)
opt:bind_key("mod-S-comma", function()
    Monitor.get_focused.ws_id = 0
    -- action.async_execute(function()
    --     local ws = Workspace.get_focused()
    --     local focus_stack = ws.focus_stack
    --
    --     local str = ""
    --     for i,con in ipairs(focus_stack) do
    --         str = str .. con.app_id .. "\n"
    --     end
    --     local dmenu = "rofi -dmenu"
    --     local echo = 'echo "' .. str .. '"'
    --     local pipe = "|"
    --     local cmd = echo .. pipe .. dmenu
    --     print(cmd)
    --     local handle = io.popen(cmd)
    --     local result = handle:read("*a")
    --     handle:close()
    --
    --      -- find container
    --     local s_con = nil
    --     for i,con in ipairs(focus_stack) do
    --         local res = result:gsub("%s+", "")
    --         local app_id = con.app_id:gsub("%s+", "")
    --         if res == app_id then
    --             s_con = con
    --             break;
    --         end
    --     end
    --
    --     if s_con then
    --         local ws = Workspace.get_focused()
    --         local ws_id = s_con.workspace:get_id()
    --         ws.tags = (1 << ws_id)
    --         ws.stack:repush(1, 0)
    --     end
    -- end)
end)
opt:bind_key("mod-comma",     function()
    Workspace.get_focused().focus_set.focus_stack_normal:repush(1, 2)
end)
opt:bind_key("mod-S-Return",  function() action.exec(termcmd) end)
opt:bind_key("mod-a",         function() action.inc(Layout.get_focused().n_master) end)
opt:bind_key("mod-x",         function() action.decrease_nmaster() end)
opt:bind_key("mod-k",         function() action.focus_on_stack(-1) end)
opt:bind_key("mod-j",         function() action.focus_on_stack(1) end)
opt:bind_key("mod-S-j",       function() action.focus_on_hidden_stack(0) end)
opt:bind_key("mod-S-k",       function() action.focus_on_hidden_stack(-1) end)
opt:bind_key("mod-tab",       function() action.swap_on_hidden_stack(0) end)
opt:bind_key("mod-S-tab",     function() action.swap_on_hidden_stack(-1) end)
opt:bind_key("mod-S-c",       function()
    if Container.get_focused() then
        action.kill(Container.get_focused())
    end
end)
opt:bind_key("mod-space",     function() Layout.load_next_in_set("default") end)
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
opt:bind_key("mod-1",         function() exec_keycombo(1) end)
opt:bind_key("mod-2",         function() exec_keycombo(2) end)
opt:bind_key("mod-3",         function() exec_keycombo(3) end)
opt:bind_key("mod-4",         function() exec_keycombo(4) end)
opt:bind_key("mod-5",         function() exec_keycombo(5) end)
opt:bind_key("mod-6",         function() exec_keycombo(6) end)
opt:bind_key("mod-7",         function() exec_keycombo(7) end)
opt:bind_key("mod-8",         function() exec_keycombo(8) end)
opt:bind_key("mod-9",         function() exec_keycombo(9) end)
opt:bind_key("mod-0",         function() Workspace.get_focused().tags = 1 << Workspace.get_focused():get_id() end)
opt:bind_key("mod-S-0",       function()
    local con = Container.get_focused()
    if con then
        Workspace.get_focused().tags = 1 << con.workspace:get_id()
    end
end)
opt:bind_key("mod-S-9",       function()
    if Container.get_focused() then
        Container.get_focused().workspace = Workspace.get(9)
    end
end)
opt:bind_key("mod-s", function() print("notice me") end)
opt:bind_key("mod-s", function() print("no") end)
opt:bind_key("mod-C-S-0",     function() Container.get_focused():set_sticky_restricted(0) end)
opt:bind_key("mod-C-S-9",     function() Container.get_focused():set_sticky_restricted(255) end)
