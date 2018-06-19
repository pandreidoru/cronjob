#
# Created by Andrei - Doru Pata on 16/06/18.
#
cmake_minimum_required(VERSION 3.11)
project(cronjob)

set(CMAKE_CXX_STANDARD 11)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-modules/)

include(EnableExtraCompilerWarnings)
globally_enable_extra_compiler_warnings()

add_subdirectory(test/src)
add_subdirectory(src)