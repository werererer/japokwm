srcs = files(
  'src/client.c',
  'src/parseConfig.c',
  'src/juliawm.c',
  'src/coreUtils.c',
  'src/xdg-shell-protocol.c',
  'src/parseConfigUtils.c',
  'src/debug.c',
  'src/tile/tile.c',
  'src/tile/tileUtils.c',
  )

inc = include_directories('include')
shared_library('juliawm',
              srcs,
              dependencies: deps,
              include_directories: inc,
              link_args: '-ljulia'
              )
