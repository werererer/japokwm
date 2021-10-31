local layouts = {"two_pane", "three_columns"}
server.default_layout_ring = Ring.new(layouts)

opt:bind_key("mod-S-b", function() Layout.toggle("monocle") end)
