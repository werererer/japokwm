# unit test setup
extraLibs = [dependency('check')]
c_args = ['-I../include']

tests = files(
    'container_test.c',
    'tile/tileUtils_test.c',
    'utils/coreUtils_test.c',
    'utils/gapUtils_test.c',
    'utils/stringUtils_test.c',
    'workspace_test.c',
    )

foreach test_file: tests
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

    # delete last part of the filename which is usally _test
    components = test_file_name.split('_')
    result = []
    foreach component : components
        if (component == components[components.length()-1])
            continue
        endif
        result = result + [component]
    endforeach
    test_name = '_'.join(result)

    test(test_name, t)
endforeach
