#
# Created by Andrei - Doru Pata on 16/06/18.
#

find_package(Boost COMPONENTS unit_test_framework REQUIRED)
if (Boost_FOUND)
    # Enable dynamic linking.
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREAD ON)

    add_executable(all_tests
                   test_entry.cc
                   job_test.cc
                   scheduler_test.cc
                   ../../inc/scheduler.h ../../src/scheduler.cc)

    target_include_directories(all_tests
                               PUBLIC
                               ${Boost_INCLUDE_DIR}
                               ${PROJECT_SOURCE_DIR}/inc
                               ${PROJECT_SOURCE_DIR}/test/inc)

    target_link_libraries(all_tests ${Boost_LIBRARIES} pthread)

    # Coverage setup
    include(CodeCoverage)
    APPEND_COVERAGE_COMPILER_FLAGS()
    set(COVERAGE_EXCLUDES
        '/usr/local/include/boost/*'
        '/usr/include/*'
        '${PROJECT_SOURCE_DIR}/test/*'
        '${PROJECT_SOURCE_DIR}/src/main.cc')

    SETUP_TARGET_FOR_COVERAGE(NAME test_coverage
                              EXECUTABLE all_tests
                              DEPENDENCIES all_tests)

    add_executable(all_tests_ASan
                   test_entry.cc
                   job_test.cc
                   scheduler_test.cc
                   ../../inc/scheduler.h ../../src/scheduler.cc)
    target_include_directories(all_tests_ASan
                               PUBLIC
                               ${Boost_INCLUDE_DIR}
                               ${PROJECT_SOURCE_DIR}/inc
                               ${PROJECT_SOURCE_DIR}/test/inc)
    target_link_libraries(all_tests_ASan ${Boost_LIBRARIES} pthread -fsanitize=address)
    target_compile_options(all_tests_ASan PUBLIC $<$<CONFIG:DEBUG>: -fsanitize=address -fPIE -pie -O1 -fno-omit-frame-pointer -g>)

    add_executable(all_tests_UBSan
                   test_entry.cc
                   job_test.cc
                   scheduler_test.cc
                   ../../inc/scheduler.h ../../src/scheduler.cc)
    target_include_directories(all_tests_UBSan
                               PUBLIC
                               ${Boost_INCLUDE_DIR}
                               ${PROJECT_SOURCE_DIR}/inc
                               ${PROJECT_SOURCE_DIR}/test/inc)
    target_link_libraries(all_tests_UBSan ${Boost_LIBRARIES} pthread -fsanitize=undefined)
    target_compile_options(all_tests_UBSan PUBLIC $<$<CONFIG:DEBUG>: -fsanitize=undefined -fsanitize-address-use-after-scope -fPIE -pie -O1 -fno-omit-frame-pointer -g>)

endif ()