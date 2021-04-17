# unit test setup
extraLibs = [dependency('check')]
c_args = ['-I../include']

test_files = files(
    'container_test.c',
    'tile/tileUtils_test.c',
    'utils/coreUtils_test.c',
    'utils/gapUtils_test.c',
    'utils/stringUtils_test.c',
    'workspace_test.c',
    )

foreach test_file: test_files
    r = run_command('basename', test_file)
    if r.returncode() != 0
      message(r.stderr().strip())
    endif
    test_file_name = r.stdout().strip()

    t = executable(test_file_name, [test_file], 
               c_args: c_args,
               dependencies: [deps, extraLibs],
               include_directories: inc,
               link_with: [wmlib],
              )

    test(test_file_name, t)
endforeach
