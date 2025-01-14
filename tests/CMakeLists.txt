include(CTest)

# All tests need to link against the core sources
file(GLOB_RECURSE SRC_CORE ../src/core/*.c)
include_directories(../src)

# Define individual test executables and add them to tests
set(SRC_TESTS test_test.c)
foreach (test_file ${SRC_TESTS})
    # get test name by removing file extension and directory path
    get_filename_component(test_name ${test_file} NAME_WE)

    add_executable(${test_name} ${test_file})
    target_sources(${test_name} PRIVATE ${SRC_CORE})
    target_link_libraries(${test_name} PUBLIC Notcurses)

    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach ()