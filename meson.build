project('japokwm', 'c', version: '0.3')
pkg = import('pkgconfig')

wayland_scanner = find_program('wayland-scanner', native: true)

root = meson.source_root()

cc = meson.get_compiler('c')

link_args = ['-lm', '-lrt']

# add version macro
version = '"@0@"'.format(meson.project_version())
git = find_program('git', native: true, required: false)
if git.found()
    git_commit = run_command([git, 'rev-parse', '--short', 'HEAD'])
    git_branch = run_command([git, 'rev-parse', '--abbrev-ref', 'HEAD'])
    if git_commit.returncode() == 0 and git_branch.returncode() == 0
        version = '"@0@-@1@ (" __DATE__ ", branch \'@2@\')"'.format(
            meson.project_version(),
            git_commit.stdout().strip(),
            git_branch.stdout().strip(),
        )
    endif
endif
add_project_arguments('-DJAPOKWM_VERSION=@0@'.format(version), language: 'c')

if get_option('xwayland')
  add_project_arguments('-DJAPOKWM_HAS_XWAYLAND=1'.format(version), language: 'c')
else
  add_project_arguments('-DJAPOKWM_HAS_XWAYLAND=0'.format(version), language: 'c')
endif

if get_option('debug')
  add_project_arguments('-DDEBUG=1'.format(version), language: 'c')
else
  add_project_arguments('-DDEBUG=0'.format(version), language: 'c')
endif


datadir = get_option('datadir')
install_data(
    'japokwm.desktop',
    install_dir: join_paths(datadir, 'wayland-sessions')
)

# generate needed protocol files
src_dir = 'src/'
include_dir = 'include/'
include_dirs = include_directories(include_dir, '/usr/include/json-c/')

scdoc = dependency('scdoc', version: '>=1.9.2', native: true)
if scdoc.found()
    scdoc_prog = find_program(scdoc.get_pkgconfig_variable('scdoc'), native: true)
    sh = find_program('sh', native: true)
    mandir = get_option('mandir')
    man_files = [
        'man/japokwm-cheatsheet.5.scd',
        'man/japokmsg.1.scd',
        'man/japokwm-action.5.scd',
        'man/japokwm-bitset.5.scd',
        'man/japokwm-color.5.scd',
        'man/japokwm-container.5.scd',
        'man/japokwm-cursor.5.scd',
        'man/japokwm-cursor_mode.5.scd',
        'man/japokwm-direction.5.scd',
        'man/japokwm-event_handler.5.scd',
        'man/japokwm-focus_set.5.scd',
        'man/japokwm-geom.5.scd',
        'man/japokwm-info.5.scd',
        'man/japokwm-layout.5.scd',
        'man/japokwm-list.5.scd',
        'man/japokwm-monitor.5.scd',
        'man/japokwm-options.5.scd',
        'man/japokwm-output_transform.5.scd',
        'man/japokwm-ring_buffer.5.scd',
        'man/japokwm-root.5.scd',
        'man/japokwm-server.5.scd',
        'man/japokwm-tag.5.scd',
        'man/japokwm.1.scd',
        'man/japokwm.5.scd',
    ]
    foreach filename : man_files
        topic = filename.split('.')[-3].split('/')[-1]
        section = filename.split('.')[-2]
        output = '@0@.@1@'.format(topic, section)

        custom_target(
            output,
            input: filename,
            output: output,
            command: [
                sh, '-c', '@0@ < @INPUT@ > @1@'.format(scdoc_prog.path(), output)
            ],
            install: true,
            install_dir: '@0@/man@1@'.format(mandir, section)
        )
    endforeach
endif

bash_comp = dependency('bash-completion', required: false)
fish_comp = dependency('fish', required: false)

if get_option('zsh-completions')
    zsh_files = files(
        'completions/zsh/_japokwm',
        'completions/zsh/_japokmsg',
    )
    zsh_install_dir = join_paths(datadir, 'zsh', 'site-functions')

    install_data(zsh_files, install_dir: zsh_install_dir)
endif

if get_option('bash-completions')
    bash_files = files(
        'completions/bash/japokwm',
        'completions/bash/japokmsg',
    )

    if bash_comp.found()
        bash_install_dir = bash_comp.get_variable(
            pkgconfig: 'completionsdir',
            pkgconfig_define: ['datadir', datadir]
        )
    else
        bash_install_dir = join_paths(datadir, 'bash-completion', 'completions')
    endif

    install_data(bash_files, install_dir: bash_install_dir)
endif

if get_option('fish-completions')
    fish_files = files(
        'completions/fish/japokwm.fish',
        'completions/fish/japokmsg.fish',
    )

    if fish_comp.found()
        fish_install_dir = fish_comp.get_variable(
            pkgconfig: 'completionsdir',
            pkgconfig_define: ['datadir', datadir]
        )
    else
        fish_install_dir = join_paths(datadir, 'fish', 'vendor_completions.d')
    endif

    install_data(fish_files, install_dir: fish_install_dir)
endif


commonSrcs = files(
  'common/stringop.c'
  )

wayland_server = dependency('wayland-server')
wayland_protos = dependency('wayland-protocols', version: '>=1.14')
wayland_client = dependency('wayland-client')
deps = [\
    dependency('xcb'),
    dependency('xkbcommon'),
    dependency('wayland-egl'),
    wayland_server,
    wayland_client,
    wayland_protos,
    dependency('pixman-1'),
    dependency('wlroots', version: ['>=0.14' ,'< 0.15']),
    dependency('x11'),
    dependency('json-c'),
    dependency('libnotify'),
    dependency('threads'),
    dependency('libinput'),
    dependency('libuv'),
    ]

# meson.add_install_script('install.sh')
# config files
subdir('src')
subdir('protocols')
subdir('config')
subdir('test')
subdir('japokmsg')
