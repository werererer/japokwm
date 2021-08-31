wl_protocol_dir = wayland_protos.get_pkgconfig_variable('pkgdatadir')
abs_src_dir = join_paths(meson.source_root(), src_dir)
abs_include_dir = join_paths(meson.source_root(), include_dir)

wayland_scanner_dep = dependency('wayland-scanner', required: false, native: true)
if wayland_scanner_dep.found()
    wayland_scanner = find_program(
        wayland_scanner_dep.get_pkgconfig_variable('wayland_scanner'),
        native: true,
    )
else
    wayland_scanner = find_program('wayland-scanner', native: true)
endif

protocols = [
    join_paths(wl_protocol_dir, 'stable/xdg-shell/xdg-shell.xml'),
    join_paths(wl_protocol_dir, 'unstable/xdg-output/xdg-output-unstable-v1.xml'),
    join_paths(wl_protocol_dir, 'unstable/pointer-constraints/pointer-constraints-unstable-v1.xml'),
    join_paths(wl_protocol_dir, 'unstable/tablet/tablet-unstable-v2.xml'),
    # 'idle.xml',
    # 'wlr-input-inhibitor-unstable-v1.xml',
    # 'wlr-output-power-management-unstable-v1.xml',
]

client_protocols = [
    join_paths(wl_protocol_dir, 'stable/xdg-shell/xdg-shell.xml'),
    join_paths(wl_protocol_dir, 'unstable/xdg-output/xdg-output-unstable-v1.xml'),
    'wlr-layer-shell-unstable-v1.xml',
    # 'wlr-input-inhibitor-unstable-v1.xml',
]

wl_protos_src = []
wl_protos_headers = []

foreach p : protocols
    pName = run_command('basename', p, '.xml').stdout().strip()
    postfix = '-protocol'
    cFile = abs_src_dir / pName + postfix +'.c'
    cHeader = abs_include_dir / pName + postfix + '.h'
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

foreach p : client_protocols
    pName = run_command('basename', p, '.xml').stdout().strip()
    postfix = '-protocol'
    cFile = abs_src_dir / pName + postfix +'.c'
    cHeader = abs_include_dir / pName + postfix + '.h'
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
