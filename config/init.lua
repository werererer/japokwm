-- some options you might want to change
opt.automatic_workspace_naming = true;
opt.sloppy_focus = true
opt.workspaces = {"0:1", "1:2", "2:3", "3:4", "4:5", "5:6", "6:7", "7:8"}
-- set it to 4 to use super instead
opt.mod = 1

local function on_start()
    -- execute programs or do what ever you want e.g.:
    -- action.exec("dbus-daemon --session --address=unix:path=$XDG_RUNTIME_DIR/bus")
    -- action.exec("xsetroot -cursor_name left_ptr")
end
-- executes function on_start when the 
event:add_listener("on_start", on_start)

local layouts = {"two_pane", "tile"}

opt.create_layout_set("default", layouts)

layout.default_layout = "two_pane"

opt:bind_key("mod-C-S-0",     function() Container.get_focused().set_sticky_restricted(0) end)
opt:bind_key("mod-C-S-9",     function() Container.get_focused():set_sticky_restricted(255) end)
