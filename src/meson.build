add_project_arguments('-g3', '-DWLR_USE_UNSTABLE', '-Werror', '-Wno-uninitialized', 
                     '-I/usr/include/julia', language: 'c')

deps = [\
    dependency('wlroots'),
    dependency('xcb'),
    dependency('xkbcommon'),
    dependency('wayland-server'),
    dependency('cairo')
    ]

subdir('core/')
executable(
    'debug',
    'debug.c',
    link_args: '-ljulia',
    install: true,
    install_dir: root / srcDir,
    )
