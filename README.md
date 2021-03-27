# japokwm - tiling made easy [![CircleCI](https://github.com/werererer/japokwm/actions/workflows/Test.yml/badge.svg)](https://github.com/werererer/japokwm/actions/workflows/Test.yml)
## Features:
- Gaps!
- Damage Tracking
- Create any layout you want with a 3 dimensional Array:
![](edit_layout.gif)
- Layout specific configs

## Known limitations and issues
japokwm is in it's alpha stage and bug may accour (If you find them please report them here)

## TODO
- unittests

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
## Acknowledgements
japokwm forked dwl and uses wlroots to do the heavy lifting
