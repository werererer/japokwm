srcDir = 'src/'
includeDir = 'include/'
inc = include_directories(includeDir, '/usr/include/json-c/')

foreach p:protocols
    pName = run_command('basename', p, '.xml').stdout().strip()
    postfix = '-protocol'
    cFile = srcDir / pName + postfix +'.c'
    cHeader = includeDir / pName + postfix + '.h'
    # if cFile doesn't exist create Source
    if run_command('[', '-f', cFile, ']').returncode() != 0
        run_command(wayland_scanner, 'private-code', p, cFile)
        message('generate: ' + cFile)
    endif
    # if cHeader doesn't exist create Header
    if run_command('[', '-f', cHeader, ']').returncode() != 0
        run_command(wayland_scanner, 'server-header', p, cHeader)
        message('generate: ' + cHeader)
    endif
endforeach

main = files('src/main.c')
srcs = files(
  'src/main.c',
  'src/monitor.c',
  'src/layout.c',
  'src/wlr-layer-shell-unstable-v1-protocol.c',
  'src/client.c',
  'src/server.c',
  'src/parseConfig.c',
  'src/utils/gapUtils.c',
  'src/utils/writeFile.c',
  'src/utils/coreUtils.c',
  'src/utils/stringUtils.c',
  'src/utils/parseConfigUtils.c',
  'src/xdg-shell-protocol.c',
  'src/tagset.c',
  'src/render/render.c',
  'src/tile/tileUtils.c',
  'src/tile/tileTexture.c',
  'src/tile/tile.c',
  'src/wlr-foreign-toplevel-management-unstable-v1-protocol.c',
  'src/keybinding.c',
  'src/translationLayer.c',
  'src/actions.c',
  'src/info.c',
  'src/ipc-json.c',
  'src/root.c',
  'src/ipc-server.c',
  'src/command.c',
  'src/stringop.c',
)
libName = 'juliawm'
wmlib = executable(libName,
              [main, srcs],
              dependencies: deps,
              include_directories: inc,
              link_args: ['-lm'],
              install: true,
              install_dir: root / srcDir,
              )
subdir('test')
