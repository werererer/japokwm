require "defaultConfig"

Min_width = 0
Max_width = 1
Min_height = 0
Max_height = 1
Min_main_width = 0.1
Max_main_width = 1
Min_main_height = 0.1
Max_main_height = 1

sloppy_focus = true
border_px = 2
root_color = {0.3, 0.3, 0.3, 1.0}
border_color = {0.0, 0.0, 1.0, 1.0}
focus_color = {1.0, 0.0, 0.0, 0.0}
overlay_color = {0.65, 0.65, 0.65, 0.9}
text_color = {0.003, 0.003, 0.003, 1.0}
sel_overlay_color = {}
sel_text_color = {}
outer_gap = 20
inner_gap = 20

tag_names = {"0:1", "1:2", "2:3", "3:4", "4:5", "5:6", "6:7", "7:8"}

-- where to put things
rules = {
    {"termite", "termite", 1, true, 3}
}

Layouts = {
    {"[M]", function() Load_layout("master") end},
    {"[]=", function() Load_layout("two_pane") end},
    {"||",  function() Load_layout("monocle") end},
    {"--",  function() Load_layout("tmp") end },
}

default_layout = Layouts[1]

monrules = {
    -- name mfact nmaster scale layout transform
    { "rule", 0.55, 1, 1, Layouts[1], NORMAL },
}

xkb_rules = {}
repeat_rate = 25
repeat_delay = 600
termcmd = "/usr/bin/termite"

Mod = Mod1
-- maps (between 1 and 4)
Keys = {
    {Mod.." "..Shift.." Return",           function(n) action.spawn(termcmd) end},
    -- {mod.." period",      function(n) focusmon(1) end},
    -- {mod.." comma",       function(n) focusmon(-1) end},
    {Mod.." a",           function(n) action.set_nmaster(2) end},
    {Mod.." x",           function(n) action.set_nmaster(1) end},
    -- {mod.." a",           function(n) action.set_tabcount(action.get_tabcount()+1) end},
    -- {mod.." x",           function(n) action.set_tabcount(action.get_tabcount()-1) end},
    {Mod.." k",           function(n) action.focus_on_stack(-1) end},
    {Mod.." j",           function(n) action.focus_on_stack(1) end},
    {Mod.." "..Shift.." j",    function(n) action.focus_on_hidden_stack(1) end},
    {Mod.." "..Shift.." k",    function(n) action.focus_on_hidden_stack(-1) end},
    -- {mod.." d",           function(n) incnmaster(-1) end},
    {Mod.." "..Shift.." c",           function(n) action.kill() end},
    {Mod.." "..Shift.." q",           function(n) action.quit() end},
    {Mod.." space",       function(n) Set_layout() end},
    {Mod.." m",           function(n) Set_layout(1) end},
    {Mod.." "..Shift.." t",           function(n) Set_layout(2) end},
    {Mod.." w",           function(n) Set_layout(3) end},
    {Mod.." b",           function(n) action.toggle_consider_layer_shell() end},
    {Mod.." "..Shift.." w",           function(n) Set_layout(4) end},
    {Mod.." "..Shift.." h",           function(n) Resize_main_all(-1/10, Resize_direction) end},
    {Mod.." "..Shift.." l",           function(n) Resize_main_all(1/10, Resize_direction) end},
    {Mod.." "..Shift.." s",    function(n) action.write_this_overlay("tmp") end},
    -- {mod.." parenright",  function(n) tag(~0) end},
    -- {mod.." greater",     function(n) tagmon(1) end},
    -- {mod.." less",        function(n) tagmon(-1) end},
    {Mod.." Return",      function(n) action.zoom() end},
    {Mod.." s",           function(n) toggle_overlay() end},
    {Mod.." 1",           function(n) action.view(0) end},
    {Mod.." 2",           function(n) action.view(1) end},
    {Mod.." 3",           function(n) action.view(2) end},
    {Mod.." 4",           function(n) action.view(3) end},
    {Mod.." 5",           function(n) action.view(4) end},
    {Mod.." 6",           function(n) action.view(5) end},
    {Mod.." 7",           function(n) action.view(6) end},
    {Mod.." 8",           function(n) action.view(7) end},
    {Mod.." 9",           function(n) action.view(8) end},
    {Mod.." "..Shift.." r",    function(n) config.reload() end},
    {Mod.." t",  function(n) action.set_floating(false)    end},
}

Buttons = {
    {Mod.." "..Btn_left,    function(n) action.move_resize(Cursor_mode.CUR_MOVE) end},
    {Mod.." "..Btn_right,   function(n) action.move_resize(Cursor_mode.CUR_RESIZE) end},
}
