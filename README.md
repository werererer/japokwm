# juliawm - for Wayland [![Build Status](https://travis-ci.org/werererer/juliawm.svg?branch=master)](https://travis-ci.org/werererer/juliawm)

juliawm tries to fill the gap of missing tiling windowmanagers that are list/stack based in wayland.
Its goals are:

- Easy to understand config and easy to hack on config
- sane default settings

Some properties that will be inherited from wlroots and the original dwl:
- Any features provided by dwm/Xlib: simple window borders, tags, keybindings, client rules, mouse move/resize (see below for why the built-in status bar is a possible exception)
- Configurable multi-monitor layout support, including position and rotation
- Configurable HiDPI/multi-DPI support
- Wayland protocols needed for daily life in the tiling world: at a minimum, xdg-shell and layer-shell (for bars/menus).  Protocols trivially provided by wlroots may also be added.
- XWayland support as provided by wlroots
- Zero flickering - Wayland users naturally expect that "every frame is perfect"
- Basic yes/no damage tracking to avoid needless redraws (if it can be done simply and has an impact on power consumption)

## Building
Execute:
```
meson build
cd build
meson compile
meson install
```
## Running
go to src/ and run `julia main.jl`

## Goals
- create your own tiling layouts easily

## Known limitations and issues
juliawm is in its very early stage and is not yet usable.

## Acknowledgements
juliawm forked dwl so that I have to write less and since the guys at dwl made pretty good groundwork.
