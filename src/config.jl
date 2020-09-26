include("defaultConfig.jl")
include("layouts/layouts.jl")
import Main.Layouts

sloppyfocus = 1
borderpx = 1
rootcolor = [0.3, 0.3, 0.3, 1.0]
bordercolor = [0.3, 0.3, 0.3, 1.0]
focuscolor = [1.0, 0.0, 0.0, 0.0]

tags = [ "1", "2", "3", "4", "5", "6", "7", "8", "9" ]

# where to put things
rules = [
    [ "Gimp", "title", 1, true, 3]
]

layouts = [
    [ "[]=", ()-> Layouts.tile() ],
    [ "><>", ()-> Layouts.floating() ],
    [ "[M]", ()-> Layouts.monocle() ],
]

monrules = [
    # name mfact nmaster scale layout transform
    [ "rule", 0.55, 1, 1, layouts[1], NORMAL ],
]


xkb_rules = []
repeatRate = 25
repeatDelay = 600
termcmd = `alacritty`

mod = mod4
#maps (between 1 and 4)
keys = [
    [mod*shift*"Return",      ()->  `$termcmd`             ],
    [mod*shift*"period",      ()->  focusmon(+1)           ],
    [mod*shift*"comma",       ()->  focusmon(-1)           ],
    [mod*shift*"k",           ()->  focusstack(-1)         ],
    [mod*shift*"j",           ()->  focusstack(1)          ],
    [mod*shift*"i",           ()->  incnmaster(+1)         ],
    [mod*shift*"d",           ()->  incnmaster(-1)         ],
    [mod*shift*"C",           ()->  killclient()           ],
    [mod*shift*"Q",           ()->  quit(0)                ],
    [mod*shift*"space",       ()->  setlayout()            ],
    [mod*shift*"t",           ()->  setlayout(layouts[0])  ],
    [mod*shift*"f",           ()->  setlayout(layouts[1])  ],
    [mod*shift*"m",           ()->  setlayout(layouts[2])  ],
    [mod*shift*"l",           ()->  setmfact(+0.05)        ],
    [mod*shift*"h",           ()->  setmfact(-0.05)        ],
    [mod*shift*"parenright",  ()->  tag(~0)                ],
    [mod*shift*"greater",     ()->  tagmon(+1)             ],
    [mod*shift*"less",        ()->  tagmon(-1)             ],
    [mod*shift*"space",       ()->  togglefloating(0)      ],
    [mod*shift*"Tab",         ()->  view()                 ],
    [mod*shift*"0",           ()->  view(~0)               ],
    [mod*shift*"Return",      ()->  zoom()                 ],
]
