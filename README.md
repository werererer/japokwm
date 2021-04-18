# japokwm <img src="japokwm_logo.png" width="50" height="50">
[![CircleCI](https://github.com/werererer/japokwm/actions/workflows/Test.yml/badge.svg)](https://github.com/werererer/japokwm/actions/workflows/Test.yml)

Japokwm is a dynamic tiling wayland compositor where you are able to create new layouts without the hassle of editing the source code. You just give it information about where windows go and it will handle stuff such as resizing all by itself --- Join the official subreddit www.reddit.com/r/japokwm.
## Features:
- Gaps!
- Damage Tracking
- Create any layout you want with a 3 dimensional Array:
![](edit_layout.gif)
- Layout specific configs

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
japokwm is in it's alpha stage and bugs may accour (If you find them please report them here)

## TODO
- unittests

## Acknowledgements
japokwm forked dwl and uses wlroots to do the heavy lifting
