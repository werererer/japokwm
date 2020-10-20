project('juliawm', 'c')
pkg = import('pkgconfig')

wayland_scanner = find_program('wayland-scanner', native: true)

# this variable will be initialized in protocols dir
protocolsDir = 'protocols'
protocols = files(
    protocolsDir + '/wlr-layer-shell-unstable-v1.xml',
    protocolsDir + '/xdg-shell.xml',
    )
subdir('src')
