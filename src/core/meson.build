srcs = files(
  'src/client.c',
  'src/parseConfig.c',
  'src/juliawm.c',
  'src/coreUtils.c',
  'src/xdg-shell-protocol.c',
  'src/parseConfigUtils.c',
  'src/tile/tile.c',
  'src/tile/tileUtils.c',
  'src/tile/tileTexture.c',
  'src/render/render.c',
  )
srcDir = 'src/'
includeDir = 'include/'
inc = include_directories(includeDir)

foreach p:protocols
    pName = run_command('basename', p, '.xml').stdout().strip()
    cFile = srcDir + pName + '.c'
    cHeader = includeDir + pName + '.h'
    # if cFile doesn't exist create Source
    if run_command('[', '-f', cFile, ']').returncode() != 0
        run_command(wayland_scanner, 'private-code', cFile)
        message('generate: ' + cFile)
    endif
    # if cHeader doesn't exist create Header
    if run_command('[', '!', '-f', cHeader, ']').returncode() != 0
        run_command(wayland_scanner, 'server-header', cHeader)
        message('generate: ' + cHeader)
    endif
endforeach

libName = 'juliawm'
root = meson.source_root()
shared_library(libName,
              srcs,
              dependencies: deps,
              include_directories: inc,
              link_args: '-ljulia',
              install: true,
              install_dir: root / srcDir,
              )
