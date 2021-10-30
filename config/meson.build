configFiles = files(
  'init.lua',
)
runtimeFiles = files(
  'tile.lua',
  'tileutils.lua',
  'default_values.lua',
  )

datadir = get_option('datadir') / 'japokwm'
install_data(runtimeFiles, install_dir: datadir)
install_data(configFiles, install_dir: '/etc/japokwm')
install_subdir('layouts', install_dir: datadir)
