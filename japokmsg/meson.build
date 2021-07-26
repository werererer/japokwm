japokmsgFiles = files(
  'main.c',
  'ipc-client.c',
  'log.c',
  )

executable(
    'japokmsg',
    [commonSrcs, japokmsgFiles],
    include_directories: include_dirs,
    dependencies: [deps],
    install: true
)
