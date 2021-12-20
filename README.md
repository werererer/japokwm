# japokwm <img src="japokwm_logo.png" width="50" height="50">
[![CircleCI](https://github.com/werererer/japokwm/actions/workflows/Test.yml/badge.svg)](https://github.com/werererer/japokwm/actions/workflows/Test.yml)

Japokwm is a dynamic tiling wayland compositor where you are able to create new layouts without the hassle of editing the source code. You just give it information about where windows go and it will handle stuff such as resizing all by itself --- Join the official subreddit www.reddit.com/r/japokwm.
## Features:
- Gaps!
- Damage Tracking
- Create any layout you want with a 3 dimensional Array:
![](edit_layout.gif)
- Layout specific configs
- a client to control the windowmanager from the terminal - japokmsg based on
  swaymsg
- a dwm based tagging system instead of normal(boring) tags

## Download
  [AUR](https://aur.archlinux.org/packages/japokwm-git)

## Building
Run:
```
meson build
cd build
meson compile
meson install
```
now you can execute japokwm:
```
japokwm
```

## Known limitations and issues
With version 0.3 I consider this project to be in it's beta stage that means the focus is now on fixing bugs and less about adding new features. In this stage bugs still may accour but way less than in the alpha stage (If you find them please report them here)

## TODO
- fix bugs and inconveniences

## Acknowledgements
japokwm forked dwl and uses wlroots and parts of sway to do the heavy lifting
