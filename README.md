# japokwm - tiling made easy [![CircleCI](https://github.com/werererer/japokwm/actions/workflows/Test.yml/badge.svg)](https://github.com/werererer/japokwm/actions/workflows/Test.yml)
![](japokwm_logo.png =80x80)
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
japokwm is in it's alpha stage and bug may accour (If you find them please report them here)

## TODO
- unittests

## Acknowledgements
japokwm forked dwl and uses wlroots to do the heavy lifting
