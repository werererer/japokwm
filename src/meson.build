add_project_arguments('-DWLR_USE_UNSTABLE', '-g3', language: 'c')

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
