# japokwm - tiling made easy [![CircleCI](https://circleci.com/gh/werererer/japokwm.svg?style=shield)](https://app.circleci.com/pipelines/github/werererer)

## Features:
- you can create any layout you want by editing a 3 dimensional Array:
![](edit_layout.gif)
- layout specific configurations
- sane default settings
## Known limitations and issues
japokwm is not ready for use yet.

## TODO
- fix a lot of annoying bugs
- write man pages / documentation
- test it on other machines
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
japokwm forked dwl and uses to do the heavy lifting
