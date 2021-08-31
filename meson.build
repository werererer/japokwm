project('japokwm', 'c', version: '0.3')
pkg = import('pkgconfig')

wayland_scanner = find_program('wayland-scanner', native: true)

root = meson.source_root()

cc = meson.get_compiler('c')

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
    dependency('wlroots', version: '>=0.13'),
    dependency('x11'),
    dependency('json-c'),
    dependency('libnotify'),
    dependency('threads'),
    dependency('libinput'),
    ]

# meson.add_install_script('install.sh')
# config files
subdir('src')
subdir('protocols')
subdir('config')
subdir('test')
subdir('japokmsg')
