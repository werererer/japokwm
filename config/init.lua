require "defaultConfig"

Min_width = 0
Max_width = 1
Min_height = 0
Max_height = 1
Min_main_width = 0.1
Max_main_width = 1
Min_main_height = 0.1
Max_main_height = 1

Sloppy_focus = true
Border_px = 2
Root_color = {0.3, 0.3, 0.3, 1.0}
Border_color = {0.0, 0.0, 1.0, 1.0}
Focus_color = {1.0, 0.0, 0.0, 0.0}
Overlay_color = {0.65, 0.65, 0.65, 0.9}
Text_color = {0.003, 0.003, 0.003, 1.0}
Sel_overlay_color = {}
Sel_text_color = {}

config.set_gaps(20, 20)

Tag_names = {"0:1", "1:2", "2:3", "3:4", "4:5", "5:6", "6:7", "7:8"}

-- where to put things
Rules = {
    -- {"termite", "termite", function(n) container.container_setsticky(n, true) end},
}

Layouts = {
    {"[M]", "master"},
    {"[]=", "two_pane"},
    {"||",  "monocle"},
    {"--",  "tmp" },
}

Default_layout = Layouts[1]

Monrules = {
    -- name mfact nmaster scale layout transform
    { "", 0.55, 1, 1, Layouts[1], Monitor_transformation.NORMAL },
}

Xkb_rules = {}
Repeat_rate = 25
Repeat_delay = 600
Termcmd = "/usr/bin/termite"

-- TODO

Mod = Mod1
-- maps (between 1 and 4)
Keys = {
    {Mod.." "..Shift.." Return",   function() action.spawn(Termcmd) end},
    -- {mod.." period",            function() focusmon(1) end},
    -- {mod.." comma",             function() focusmon(-1) end},
    {Mod.." a",                    function() action.set_nmaster(2) end},
    {Mod.." x",                    function() action.set_nmaster(1) end},
    -- {mod.." a",                 function() action.set_tabcount(action.get_tabcount()+1) end},
    -- {mod.." x",                 function() action.set_tabcount(action.get_tabcount()-1) end},
    {Mod.." k",                    function() action.focus_on_stack(-1) end},
    {Mod.." j",                    function() action.focus_on_stack(1) end},
    {Mod.." "..Shift.." j",        function() action.focus_on_hidden_stack(1) end},
    {Mod.." "..Shift.." k",        function() action.focus_on_hidden_stack(-1) end},
    -- {mod.." d",                 function() incnmaster(-1) end},
    {Mod.." "..Shift.." c",        function() action.kill() end},
    {Mod.." "..Shift.." q",        function() action.quit() end},
    {Mod.." space",                function() Set_layout() end},
    {Mod.." m",                    function() Set_layout(1) end},
    {Mod.." "..Shift.." t",        function() Set_layout(2) end},
    {Mod.." w",                    function() Set_layout(3) end},
    {Mod.." b",                    function() action.toggle_consider_layer_shell() end},
    {Mod.." "..Shift.." w",        function() Set_layout(4) end},
    {Mod.." "..Shift.." h",        function() action.resize_main(-1/10) end},
    {Mod.." "..Shift.." l",        function() action.resize_main(1/10) end},
    {Mod.." "..Shift.." s",        function() action.write_this_overlay("tmp") end},
    -- {mod.." parenright",        function() tag(~0) end},
    -- {mod.." greater",           function() tagmon(1) end},
    -- {mod.." less",              function() tagmon(-1) end},
    {Mod.." Return",               function() action.zoom() end},
    {Mod.." 1",                    function() action.view(0) end},
    {Mod.." 2",                    function() action.view(1) end},
    {Mod.." 3",                    function() action.view(2) end},
    {Mod.." 4",                    function() action.view(3) end},
    {Mod.." 5",                    function() action.view(4) end},
    {Mod.." 6",                    function() action.view(5) end},
    {Mod.." 7",                    function() action.view(6) end},
    {Mod.." 8",                    function() action.view(7) end},
    {Mod.." 9",                    function() action.view(8) end},
    {Mod.." "..Shift.." 1",        function() action.move_client_to_workspace(0) end},
    {Mod.." "..Shift.." 2",        function() action.move_client_to_workspace(1) end},
    {Mod.." "..Shift.." 3",        function() action.move_client_to_workspace(2) end},
    {Mod.." "..Shift.." 4",        function() action.move_client_to_workspace(3) end},
    {Mod.." "..Shift.." 5",        function() action.move_client_to_workspace(4) end},
    {Mod.." "..Shift.." 6",        function() action.move_client_to_workspace(5) end},
    {Mod.." "..Shift.." 7",        function() action.move_client_to_workspace(6) end},
    {Mod.." "..Shift.." 8",        function() action.move_client_to_workspace(7) end},
    {Mod.." "..Shift.." 9",        function() action.move_client_to_workspace(8) end},
    {Mod.." "..Shift.." r",        function() config.reload() end},
    {Mod.." t",                    function() action.set_floating(false)    end},
}

Buttons = {
    {Mod.." "..Btn_left,    function() action.move_resize(Cursor_mode.CUR_MOVE) end},
    {Mod.." "..Btn_right,   function() action.move_resize(Cursor_mode.CUR_RESIZE) end},
}
