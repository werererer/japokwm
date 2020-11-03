module config
include("defaultConfig.jl")

sloppyFocus = true
borderPx = 1
rootColor = [0.3, 0.3, 0.3, 1.0]
borderColor = [0.3, 0.3, 0.3, 1.0]
focusColor = [1.0, 0.0, 0.0, 0.0]
overlayColor = [0.65, 0.65, 0.65, 0.5]
textColor = [0.003, 0.003, 0.003, 1.0]
selOverlayColor = []
selTextColor = []
outerGap = 30
innerGap = 20

tagNames = [ "1", "2", "3", "4", "5", "6", "7", "8", "9" ]

# where to put things
rules = [
    [ "Gimp", "title", 1, true, 3]
]

layouts = [
    [ "[M]", (n)-> Layouts.monocle(n) ],
    [ "[]=", (n)-> Layouts.tile(n) ],
    [ "[]=", (n)-> Layouts.twoPane(n) ],
    [ "><>", (n)-> Layouts.floating(n) ],
]

defaultLayout = layouts[1]

monrules = [
    # name mfact nmaster scale layout transform
    [ "rule", 0.55, 1, 1, layouts[1], NORMAL ],
]

xkb_rules = []
repeatRate = 25
repeatDelay = 600
termcmd = "/usr/bin/termite"

mod = mod1
#maps (between 1 and 4)
keys = [
        ["$mod u",           ()->  spawn(termcmd)        ],
        ["$mod period",      ()->  focusmon(+1)           ],
        ["$mod comma",       ()->  focusmon(-1)           ],
        ["$mod k",           ()->  focusOnStack(-1)         ],
        ["$mod j",           ()->  focusOnStack(1)          ],
        ["$mod $shift j",    ()->  focusOnHiddenStack(1)          ],
        ["$mod $shift k",    ()->  focusOnHiddenStack(-1)          ],
        ["$mod d",           ()->  incnmaster(-1)         ],
        ["$mod c",           ()->  killclient()           ],
        ["$mod q",           ()->  quit()                 ],
        ["$mod p",           ()->  Layouts.splitThisContainer(1/2)                 ],
        ["$mod o",           ()->  Layouts.vsplitThisContainer(1/2)                 ],
        ["$mod i",           ()->  Layouts.mergeContainer(1, 1, 2)                 ],
        ["$mod space",       ()->  setLayout()            ],
        ["$mod m",           ()->  setLayout(1)  ],
        ["$mod t",           ()->  setLayout(2)  ],
        ["$mod l",           ()->  Layouts.resizeThisAll(1/10, Layouts.RIGHT)        ],
        ["$mod h",           ()->  Layouts.resizeThisAll(1/10, Layouts.LEFT)        ],
        ["$mod $shift s",    ()->  writeThisOverlay("testLayout")        ],
        ["$mod parenright",  ()->  tag(~0)                ],
        ["$mod greater",     ()->  tagmon(+1)             ],
        ["$mod less",        ()->  tagmon(-1)             ],
        ["$mod Return",      ()->  zoom()                 ],
        ["$mod s",           ()->  toggleOverlay()                 ],
        ["$mod 1",           ()->  view(1)                 ],
        ["$mod 2",           ()->  view(2)                 ],
        ["$mod 3",           ()->  view(4)                 ],
        ["$mod 4",           ()->  view(8)                 ],
        ["$mod 5",           ()->  view(16)                 ],
        ["$mod 6",           ()->  view(32)                 ],
        ["$mod 7",           ()->  view(64)                 ],
        ["$mod 8",           ()->  view(128)                 ],
        ["$mod 9",           ()->  view(256)                 ],
        ["$mod 0",           ()->  view(511)                 ],
        ["$mod $shift 1",           ()->  toggleAddView(1)                 ],
        ["$mod $shift 2",           ()->  toggleAddView(2)                 ],
        ["$mod $shift 3",           ()->  toggleAddView(4)                 ],
        ["$mod $shift 4",           ()->  toggleAddView(8)                 ],
        ["$mod $shift 5",           ()->  toggleAddView(16)                 ],
        ["$mod $shift 6",           ()->  toggleAddView(32)                 ],
        ["$mod $shift 7",           ()->  toggleAddView(64)                 ],
        ["$mod $shift 8",           ()->  toggleAddView(128)                 ],
        ["$mod $shift 9",           ()->  toggleAddView(256)                 ],
       ]

buttons = [
           ["$mod $btnLeft",    ()  ->  moveResize(CurMove)      ],
           ["$mod $btnMiddle",  ()  ->  toggleFloating()  ],
           ["$mod $btnRight",   ()  ->  moveResize(CurResize)      ],
          ]
end
