project('juliawm', 'c')
pkg = import('pkgconfig')

wayland_scanner = find_program('wayland-scanner', native: true)

# this variable will be initialized in protocols dir
localProtocolsDir = 'protocols'
protocolsDir = run_command(['pkg-config', '--variable=pkgdatadir', 
    'wayland-protocols']).stdout().strip()
protocols = files(
    localProtocolsDir + '/wlr-layer-shell-unstable-v1.xml',
    protocolsDir + '/stable/xdg-shell/xdg-shell.xml',
    )
subdir('src')
