# unit test setup
extraLibs = [dependency('check')]
c_args = ['-I../include']
link_args = ['-lm']

t = executable('container_test', ['container_test.c', srcs], 
               c_args: c_args,
               link_args: link_args,
               dependencies: [deps, extraLibs],
               include_directories: inc,
              )
test('container', t)
# t = executable('tagset_test', ['tagset_test.c', srcs], 
#                c_args: c_args,
#                link_args: link_args,
#                dependencies: [deps, extraLibs],
#                link_with: [wmlib],
#                include_directories: inc,
#               )
# test('tagset', t)
# t = executable('stringUtils_test', ['utils/stringUtils_test.c', srcs], 
#                c_args: c_args,
#                link_args: link_args,
#                dependencies: [deps, extraLibs],
#                link_with: [wmlib],
#                include_directories: inc,
#               )
# test('stringUtils', t)
t = executable('gapUtils_test', ['utils/gapUtils_test.c', srcs], 
               c_args: c_args,
               link_args: link_args,
               dependencies: [deps, extraLibs],
               include_directories: inc,
              )
test('gapUtils', t)
t = executable('tileUtils_test', ['tile/tileUtils_test.c', srcs], 
               c_args: c_args,
               link_args: link_args,
               dependencies: [deps, extraLibs],
               include_directories: inc,
              )
test('tileUtils', t)
