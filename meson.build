project('japokwm', 'c', version: '0.3')
pkg = import('pkgconfig')

wayland_scanner = find_program('wayland-scanner', native: true)

# this variable will be initialized in protocols dir
protocolsDir = 'protocols'
protocols = files(
    protocolsDir + '/wlr-layer-shell-unstable-v1.xml',
    protocolsDir + '/xdg-shell.xml',
    # protocolsDir + '/wlr-foreign-toplevel-management-unstable-v1.xml',
    )
root = meson.source_root()

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

datadir = get_option('datadir')
install_data(
    'japokwm.desktop',
    install_dir: join_paths(datadir, 'wayland-sessions')
)

# generate needed protocol files
srcDir = 'src/'
includeDir = 'include/'
includeDirectories = include_directories(includeDir, '/usr/include/json-c/')
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

deps = [\
    dependency('xcb'),
    dependency('xkbcommon'),
    dependency('wayland-server'),
    dependency('wayland-egl'),
    dependency('pixman-1'),
    dependency('wlroots', version: '>=0.13'),
    dependency('x11'),
    dependency('json-c'),
    dependency('libnotify'),
    dependency('threads'),
    ]

# meson.add_install_script('install.sh')
# config files
subdir('src')
subdir('config')
subdir('test')
subdir('japokmsg')
