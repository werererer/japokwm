# unit test setup
extraLibs = [dependency('check')]
c_args = ['-I..']
link_args = ['-ljulia', '-lm']

t = executable('client_test', ['client_test.c', srcs], 
               c_args: c_args,
               link_args: link_args,
               dependencies: [deps, extraLibs],
               link_with: [wmlib],
               include_directories: inc,
              )
test('test', t)
t = executable('tagset_test', ['tagset_test.c', srcs], 
               c_args: c_args,
               link_args: link_args,
               dependencies: [deps, extraLibs],
               link_with: [wmlib],
               include_directories: inc,
              )
test('tagset', t)
