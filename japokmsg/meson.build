japokmsgFiles = files(
  'main.c',
  'ipc-client.c',
  'log.c',
  )

executable(
    'japokmsg',
    [commonSrcs, japokmsgFiles],
    include_directories: includeDirectories,
    dependencies: [deps],
    install: true
)
