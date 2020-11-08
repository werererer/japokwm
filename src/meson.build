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
    ]

subdir('core/')
