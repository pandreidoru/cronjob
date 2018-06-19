#
# Created by Andrei - Doru Pata on 16/06/18.
#

find_package(Boost COMPONENTS unit_test_framework REQUIRED)
if (Boost_FOUND)
    # Enable dynamic linking.
    set(Boost_USE_STATIC_LIBS OFF)
    set(Boost_USE_MULTITHREAD ON)

    add_executable(all_tests
                   test_entry.cc
                   job_test.cc
                   ../inc/scheduler_test.h scheduler_test.cc
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

    add_executable(stress_tests
                   test_entry.cc
                   scheduler_stress_test.cpp
                   ../inc/scheduler_test.h
                   ../../inc/scheduler.h ../../src/scheduler.cc)

    target_include_directories(stress_tests
                               PUBLIC
                               ${Boost_INCLUDE_DIR}
                               ${PROJECT_SOURCE_DIR}/inc
                               ${PROJECT_SOURCE_DIR}/test/inc)

    target_link_libraries(stress_tests ${Boost_LIBRARIES} pthread)

endif ()