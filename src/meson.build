add_project_arguments('-rdynamic', '-DWLR_USE_UNSTABLE', '-Werror',
'-Wno-unused-function', '-Wno-uninitialized', '-Wno-sizeof-pointer-div', language: 'c')

deps = [\
    dependency('xcb'),
    dependency('xkbcommon'),
    dependency('wayland-server'),
    dependency('wayland-egl'),
    dependency('pixman-1'),
    dependency('wlroots', version: '>=0.13'),
    dependency('x11'),
    dependency('json-c'),
    dependency('pangocairo'),
    ]

foreach name : ['lua', 'lua5.3', 'lua-5.3', 'lua53']
    luaDep = dependency(name, version: '>=5.3', required: false)
    if luaDep.found()
        break
    endif
endforeach
if not luaDep.found()
    error('Lua could not be found!')
endif
deps += [luaDep]

main = files('main.c')
srcs = files(
    'bitset/bitset.c',
    'bitset/vector.c',
    'client.c',
    'clipboard.c',
    'command.c',
    'container.c',
    'cursor.c',
    'event_handler.c',
    'ipc-json.c',
    'ipc-server.c',
    'keybinding.c',
    'keyboard.c',
    'layer_shell.c',
    'layout.c',
    'layout_set.c',
    'lib/actions/actions.c',
    'lib/actions/libcontainer.c',
    'lib/config/config.c',
    'lib/config/localconfig.c',
    'lib/event_handler/lib_event_handler.c',
    'lib/event_handler/local_event_handler.c',
    'lib/info/info.c',
    'lib/layout/lib_layout.c',
    'lib/monitor/lib_monitor.c',
    'list_set.c',
    'main.c',
    'monitor.c',
    'options.c',
    'output.c',
    'popup.c',
    'render/render.c',
    'root.c',
    'scratchpad.c',
    'server.c',
    'stringop.c',
    'tagset.c',
    'tile/tile.c',
    'tile/tileUtils.c',
    'translationLayer.c',
    'utils/coreUtils.c',
    'utils/gapUtils.c',
    'utils/parseConfigUtils.c',
    'utils/stringUtils.c',
    'utils/writeFile.c',
    'wlr-layer-shell-unstable-v1-protocol.c',
    'wlr_signal.c',
    'workspace.c',
    'xdg-shell-protocol.c',
    'xdg_shell.c',
    'xwayland.c',
    'tileTexture.c'
  )

libName = 'japokwm_lib'
wmlib = static_library(libName,
              [srcs],
              dependencies: deps,
              include_directories: inc,
              link_args: ['-lm'],
              )
executable('japokwm',
      [main],
      dependencies: deps,
      include_directories: inc,
      link_with: [wmlib],
      install: true,
      )
