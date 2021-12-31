-- This file can be completely empty. If so then the default configuration will
-- be used.

-- mod key of 1 = alt if you want to use super use 4
opt.mod = 1

-- you can define keybindings like this:
local termcmd = "/usr/bin/alacritty"
opt:bind_key("mod-S-Return", function() Action.exec(termcmd) end)
-- print statements will be shown over notifications
opt:bind_key("mod-q", function() print("Hello world") end)

local function on_start()
    -- this function is called when the application starts
    -- you can use it to initialize your application

    -- start waybar
    -- Action.exec("waybar")

    -- change bg image
    -- Action.exec("swaybg -i ~/some/image.png")

    -- make root cursor
    -- Action.exec("xsetroot -cursor_name left_ptr")
end

-- you can find some more event listeners in japokwm-event_handlers(5)
event:add_listener("on_start", on_start)

-- set the master layout this will affect what happens if you press
-- mod-a and mod-x
layout:set_master_layout_data(
{{{0, 0, 1, 1}}, {{0, 0, 0.5, 1}, {0.5, 0, 0.5, 1}}}
)

-- You can define which layouts are switch to by default here
local layouts = {"three_columns", "two_pane", "user1"}
server.default_layout_ring = Ring.new(layouts)
opt.default_layout = layouts[1]

-- NOTE: this feature may not be stable yet
-- you can add a rule to a container like this:
-- opt:add_rule({ title="", class = "", callback = function(con) con.ratio = 0 end})
