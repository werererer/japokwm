add_project_arguments('-rdynamic', '-DWLR_USE_UNSTABLE', '-Werror',
'-Wno-unused-function', '-Wno-uninitialized', '-Wno-sizeof-pointer-div', language: 'c')

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
    'utils/vector.c',
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
    'lib/actions/lib_actions.c',
    'lib/actions/lib_container.c',
    'lib/config/lib_config.c',
    'lib/config/local_config.c',
    'lib/event_handler/lib_event_handler.c',
    'lib/event_handler/local_event_handler.c',
    'lib/info/lib_info.c',
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
  )

libName = 'japokwm_lib'
wmlib = static_library(libName,
              [srcs, commonSrcs],
              dependencies: deps,
              include_directories: includeDirectories,
              link_args: ['-lm'],
              )
executable('japokwm',
      [main],
      dependencies: deps,
      include_directories: includeDirectories,
      link_with: [wmlib],
      install: true,
      )
