local cursor_mode = {
    CUR_NORMAL = 0,
    CUR_MOVE = 1,
    CUR_RESIZE = 2,
}

-- uses wl_output_transform enum values
local monitor_transform = {
    TRANSFORM_NORMAL = 0,
    TRANSFORM_90 = 1,
    TRANSFORM_180 = 2,
    TRANSFORM_270 = 3,
    TRANSFORM_FLIPPED = 4,
    TRANSFORM_FLIPPED_90 = 5,
    TRANSFORM_FLIPPED_180 = 6,
    TRANSFORM_FLIPPED_270 = 7,
}

config.set_sloppy_focus(true)
config.set_borderpx(2)
config.set_gaps(20, 20)
config.set_root_color({0.3, 0.3, 0.3, 1.0})
config.set_focus_color({1.0, 0.0, 0.0, 1.0})
config.set_border_color({0.0, 0.0, 1.0, 1.0})
config.set_repeat_rate(25)
config.set_repeat_delay(600)

config.set_workspaces({"0:1", "1:2", "2:3", "3:4", "4:5", "5:6", "6:7", "7:8"})

-- config.set_rules({
--     -- {"termite", "termite", function(n) container.container_setsticky(n, true) end}
-- })

local layouts = {
    {"[M]", "master"},
    {"[]=", "two_pane"},
    {"||",  "monocle"},
    {"--",  "tmp" },
}

config.set_layouts(layouts)

config.set_default_layout(layouts[1])
config.set_monrules({
    { "", 0.55, 1, 1, layouts[1], monitor_transform.TRANSFORM_NORMAL },
})

local termcmd = "/usr/bin/termite"

config.set_keybinds({
    {"mod-period",    function() action.toggle_workspace() end},
    {"mod-S-period",    function() action.toggle_layout() end},
    {"mod-S-Return",  function() action.spawn(termcmd) end},
    {"mod-a",         function() action.set_nmaster(2) end},
    {"mod-x",         function() action.set_nmaster(1) end},
    {"mod-k",         function() action.focus_on_stack(-1) end},
    {"mod-j",         function() action.focus_on_stack(1) end},
    {"mod-S-j",       function() action.focus_on_hidden_stack(1) end},
    {"mod-S-k",       function() action.focus_on_hidden_stack(-1) end},
    {"mod-S-c",       function() action.kill() end},
    {"mod-S-q",       function() action.quit() end},
    {"mod-space",     function() action.load_layout() end},
    {"mod-m",         function() action.load_layout(1) end},
    {"mod-S-t",       function() action.load_layout(2) end},
    {"mod-w",         function() action.load_layout(3) end},
    {"mod-S-w",       function() action.load_layout(4) end},
    {"mod-b",         function() action.toggle_consider_layer_shell() end},
    {"mod-S-h",       function() action.resize_main(-1/10) end},
    {"mod-S-l",       function() action.resize_main(1/10) end},
    {"mod-S-s",       function() action.write_this_overlay("tmp") end},
    {"mod-Return",    function() action.zoom() end},
    {"mod-S-j",       function() action.repush(1) end},
    {"mod-1",         function() action.view(0) end},
    {"mod-2",         function() action.view(1) end},
    {"mod-3",         function() action.view(2) end},
    {"mod-4",         function() action.view(3) end},
    {"mod-5",         function() action.view(4) end},
    {"mod-6",         function() action.view(5) end},
    {"mod-7",         function() action.view(6) end},
    {"mod-8",         function() action.view(7) end},
    {"mod-9",         function() action.view(8) end},
    {"mod-S-1",       function() action.move_client_to_workspace(0) end},
    {"mod-S-2",       function() action.move_client_to_workspace(1) end},
    {"mod-S-3",       function() action.move_client_to_workspace(2) end},
    {"mod-S-4",       function() action.move_client_to_workspace(3) end},
    {"mod-S-5",       function() action.move_client_to_workspace(4) end},
    {"mod-S-6",       function() action.move_client_to_workspace(5) end},
    {"mod-S-7",       function() action.move_client_to_workspace(6) end},
    {"mod-S-8",       function() action.move_client_to_workspace(7) end},
    {"mod-S-9",       function() action.move_client_to_workspace(8) end},
    {"mod-r",         function() config.reload() end},
    {"mod-t",         function() action.set_floating(false)    end},
    -- {"mod-period",            function() focusmon(1) end},
    -- {"mod-comma",             function() focusmon(-1) end},
    -- {"mod-a",                 function() action.set_tabcount(action.get_tabcount()+1) end},
    -- {"mod-x",                 function() action.set_tabcount(action.get_tabcount()-1) end},
    -- {"mod-d",                 function() incnmaster(-1) end},
    -- {"mod-parenright",        function() tag(~0) end},
    -- {"mod-greater",           function() tagmon(1) end},
    -- {"mod-less",              function() tagmon(-1) end},
})

config.set_buttons({
    {"mod-M1",  function() action.move_resize(cursor_mode.CUR_MOVE) end},
    {"mod-M2",  function() action.move_resize(cursor_mode.CUR_RESIZE) end},
})
