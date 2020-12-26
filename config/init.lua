require "defaultConfig"

sloppy_focus = true
border_px = 2
root_color = {0.3, 0.3, 0.3, 1.0}
border_color = {0.3, 0.3, 0.3, 1.0}
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
    {"Gimp", "title", 1, true, 3}
}

layouts = {
    {"[]=", function(n) tile(n) end},
    {"[M]", function(n) monocle(n) end},
    { "gf", function(n) load_layout("tmp") end },
    {"||", function(n) two_pane(n) end},
}

default_layout = layouts[1]

monrules = {
    -- name mfact nmaster scale layout transform
    { "rule", 0.55, 1, 1, layouts[1], NORMAL },
}

xkb_rules = {}
repeat_rate = 25
repeat_delay = 600
termcmd = "/usr/bin/termite"

mod = mod1
-- maps (between 1 and 4)
keys = {
    {mod.."    "..shift.." Return",           function(n) action.spawn(termcmd) end},
    -- {mod.." period",      function(n) focusmon(1) end},
    -- {mod.." comma",       function(n) focusmon(-1) end},
    {mod.." a",           function(n) action.set_nmaster(2) end},
    {mod.." x",           function(n) action.set_nmaster(1) end},
    -- {mod.." a",           function(n) action.set_tabcount(action.get_tabcount()+1) end},
    -- {mod.." x",           function(n) action.set_tabcount(action.get_tabcount()-1) end},
    {mod.." k",           function(n) action.focus_on_stack(-1) end},
    {mod.." j",           function(n) action.focus_on_stack(1) end},
    {mod.." "..shift.." j",    function(n) action.focus_on_hidden_stack(1) end},
    {mod.." "..shift.." k",    function(n) action.focus_on_hidden_stack(-1) end},
    -- {mod.." d",           function(n) incnmaster(-1) end},
    {mod.." "..shift.." c",           function(n) action.kill() end},
    {mod.." "..shift.." q",           function(n) action.quit() end},
    {mod.." p",           function(n) split_this_container(1/2) end},
    {mod.." o",           function(n) vsplit_this_container(1/2) end},
    {mod.." i",           function(n) merge_container(1, 1, 2) end},
    {mod.." space",       function(n) set_layout() end},
    {mod.." m",           function(n) set_layout(1) end},
    {mod.." "..shift.." t",           function(n) set_layout(2) end},
    {mod.." w",           function(n) set_layout(3) end},
    {mod.." b",           function(n) action.toggle_consider_layer_shell() end},
    {mod.." "..shift.." w",           function(n) set_layout(4) end},
    {mod.." "..shift.." l",           function(n) resize_main_all(1/10, Direction.RIGHT) end},
    {mod.." "..shift.." h",           function(n) resize_main_all(-(1/10), Direction.RIGHT) end},
    {mod.." "..shift.." s",    function(n) action.write_this_overlay("tmp") end},
    -- {mod.." parenright",  function(n) tag(~0) end},
    -- {mod.." greater",     function(n) tagmon(1) end},
    -- {mod.." less",        function(n) tagmon(-1) end},
    {mod.." Return",      function(n) action.zoom() end},
    {mod.." s",           function(n) toggleOverlay() end},
    {mod.." 1",           function(n) action.view(0) end},
    {mod.." 2",           function(n) action.view(1) end},
    {mod.." 3",           function(n) action.view(2) end},
    {mod.." 4",           function(n) action.view(3) end},
    {mod.." 5",           function(n) action.view(4) end},
    {mod.." 6",           function(n) action.view(5) end},
    {mod.." 7",           function(n) action.view(6) end},
    {mod.." 8",           function(n) action.view(7) end},
    {mod.." 9",           function(n) action.view(8) end},
    {mod.." "..shift.." r",    function(n) config.reload() end},
    {mod.." t",  function(n) action.set_floating(false)    end},
}

buttons = {
    {mod.." "..btn_left,    function(n) action.move_resize(cursor_mode._cur_move) end},
    {mod.." "..btn_right,   function(n) action.move_resize(cursor_mode._cur_resize) end},
}
