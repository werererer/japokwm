add_project_arguments('-DWLR_USE_UNSTABLE', '-Werror', '-Wno-uninitialized',
                     '-I/usr/include/julia', language: 'c')

deps = [\
    dependency('xcb'),
    dependency('xkbcommon'),
    dependency('wayland-server'),
    dependency('wayland-egl'),
    dependency('wlroots'),
    dependency('cairo'),
    dependency('lua'),
    dependency('x11'),
    dependency('json-c'),
    ]

main = files('main.c')
srcs = files(
  'monitor.c',
  'layout.c',
  'wlr-layer-shell-unstable-v1-protocol.c',
  'client.c',
  'server.c',
  'parseConfig.c',
  'utils/gapUtils.c',
  'utils/writeFile.c',
  'utils/coreUtils.c',
  'utils/stringUtils.c',
  'utils/parseConfigUtils.c',
  'xdg-shell-protocol.c',
  'tagset.c',
  'render/render.c',
  'tile/tileUtils.c',
  'tile/tileTexture.c',
  'tile/tile.c',
  'wlr-foreign-toplevel-management-unstable-v1-protocol.c',
  'keybinding.c',
  'translationLayer.c',
  'actions.c',
  'info.c',
  'ipc-json.c',
  'root.c',
  'ipc-server.c',
  'command.c',
  'stringop.c',
  'popup.c',
)

libName = 'juliawm'
wmlib = executable(libName,
              [main, srcs],
              dependencies: deps,
              include_directories: inc,
              link_args: ['-lm'],
              install: true,
              )
