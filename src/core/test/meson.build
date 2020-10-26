# unit test setup
extraLibs = [dependency('cmocka')]

t = executable('t', ['main.c', srcs], 
               c_args: ['-I..'],
               link_args: ['-ljulia'],
               dependencies: [deps, extraLibs],
               link_with: [wmlib],
               include_directories: inc,
              )
test('test', t)
