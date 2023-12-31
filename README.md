# Japokwm <img src="japokwm_logo.png" width="50" height="50">
[![CircleCI](https://github.com/werererer/japokwm/actions/workflows/Test.yml/badge.svg)](https://github.com/werererer/japokwm/actions/workflows/Test.yml)

😃 Ready to show your window manager who's in charge?

Japokwm is a dynamic tiling Wayland compositor that makes it a breeze to create custom layouts with a simple configuration. 🚀 You determine where the windows should go, and Japokwm handles the rest, allowing you to sit back, relax, and enjoy the view. 🌟

--- Join the official subreddit www.reddit.com/r/japokwm or the [Discord channel](https://discord.gg/WpGjAU393M). ---

## ✨ Features:
- Gaps! ✨
- Damage Tracking 🛠️
- Layout specific configs 📝
- A client to control the window manager from the terminal - japokmsg, based on swaymsg 💼
- A dwm-based tagging system instead of normal (boring) tags 🏷️
- Create any layout you want with a 3-dimensional Array:
  ![Edit Layout](edit_layout.gif)

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


## Distribution Packages
  [AUR](https://aur.archlinux.org/packages/japokwm-git) (May be out of Date, Git is better maintained

## Known limitations and issues
With version 0.3 I consider this project to be in it's beta stage that means the focus is now on fixing bugs and less about adding new features. In this stage bugs still may occur but way less than in the alpha stage (If you find them please report them here)

## TODO
polish program and syntax for configuring

## Acknowledgements
japokwm forked dwl and uses wlroots and parts of sway to do the heavy lifting
