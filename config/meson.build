configFiles = files(
  'init.lua',
)
runtimeFiles = files(
  'tile.lua',
  'tileutils.lua',
  )

install_data(runtimeFiles, install_dir: get_option('datadir') / 'japokwm')
install_data(configFiles, install_dir: '/etc/japokwm')
install_subdir('layouts', install_dir: '/etc/japokwm')
