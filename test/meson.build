# unit test setup
extraLibs = [dependency('check')]
c_args = ['-I../include']

t = executable('container_test', ['container_test.c'], 
               c_args: c_args,
               dependencies: [deps, extraLibs],
               include_directories: inc,
               link_with: [wmlib],
              )
test('container', t)
t = executable('coreUtils_test', ['container_test.c'], 
               c_args: c_args,
               dependencies: [deps, extraLibs],
               include_directories: inc,
               link_with: [wmlib],
              )
test('coreUtils', t)
t = executable('gapUtils_test', ['utils/gapUtils_test.c'], 
               c_args: c_args,
               dependencies: [deps, extraLibs],
               include_directories: inc,
               link_with: [wmlib],
              )
test('gapUtils', t)
t = executable('tileUtils_test', ['tile/tileUtils_test.c'], 
               c_args: c_args,
               dependencies: [deps, extraLibs],
               include_directories: inc,
               link_with: [wmlib],
              )
test('tileUtils', t)
