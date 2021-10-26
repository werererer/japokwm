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
    'client.c',
    'color.c',
    'command.c',
    'container.c',
    'cursor.c',
    'event_handler.c',
    'input_manager.c',
    'ipc-json.c',
    'ipc-server.c',
    'keybinding.c',
    'keyboard.c',
    'layer_shell.c',
    'layout.c',
    'layout_set.c',
    'main.c',
    'monitor.c',
    'options.c',
    'output.c',
    'popup.c',
    'root.c',
    'scratchpad.c',
    'seat.c',
    'server.c',
    'tagset.c',
    'translationLayer.c',
    'wlr_signal.c',
    'workspace.c',
    'xdg_shell.c',
    'xwayland.c',
    'bitset/bitset.c',
    'command/commands.c',
    'lib/lib_actions.c',
    'lib/lib_bitset.c',
    'lib/lib_color.c',
    'lib/lib_container.c',
    'lib/lib_cursor.c',
    'lib/lib_cursor_mode.c',
    'lib/lib_direction.c',
    'lib/lib_event_handler.c',
    'lib/lib_focus_set.c',
    'lib/lib_geom.c',
    'lib/lib_info.c',
    'lib/lib_layout.c',
    'lib/lib_list.c',
    'lib/lib_list2D.c',
    'lib/lib_monitor.c',
    'lib/lib_options.c',
    'lib/lib_output_transform.c',
    'lib/lib_server.c',
    'lib/lib_root.c',
    'lib/lib_workspace.c',
    'lib/lib_container_property.c',
    'lib/local_event_handler.c',
    'lib/local_options.c',
    'list_sets/container_stack_set.c',
    'list_sets/focus_stack_set.c',
    'list_sets/list_set.c',
    'render/render.c',
    'rules/mon_rule.c',
    'rules/rule.c',
    'tile/tileUtils.c',
    'utils/coreUtils.c',
    'utils/gapUtils.c',
    'utils/parseConfigUtils.c',
    'utils/stringUtils.c',
    'utils/writeFile.c',
  )

libName = 'japokwm_lib'
wmlib = static_library(libName,
              [srcs, commonSrcs],
              dependencies: deps,
              include_directories: include_dirs,
              link_args: link_args,
              )
executable('japokwm',
      [main],
      dependencies: deps,
      include_directories: include_dirs,
      link_args: link_args,
      link_with: [wmlib],
      install: true,
      )
