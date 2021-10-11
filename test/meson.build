# unit test setup
c_args = ['-I../include']

test_files = files(
    'event_handler_test.c',
    'tagset_test.c',
    'container_test.c',
    'tile/tileUtils_test.c',
    'utils/coreUtils_test.c',
    'utils/gapUtils_test.c',
    'utils/stringUtils_test.c',
    'workspace_test.c',
    'scratchpad_test.c',
    'bitset_test.c',
    'keybinding_test.c'
    )

foreach test_file: test_files
    r = run_command('basename', test_file)
    if r.returncode() != 0
      message(r.stderr().strip())
    endif
    test_file_name = r.stdout().strip()

    t = executable(test_file_name, [test_file], 
               c_args: c_args,
               dependencies: [deps],
               include_directories: include_dirs,
               link_args: link_args,
               link_with: [wmlib],
              )

    test(test_file_name, t)
endforeach
