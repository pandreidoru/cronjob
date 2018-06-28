#
# Created by Andrei - Doru Pata on 18/06/18.
#
# TARGET - Main project.
add_executable(${PROJECT_NAME}
               main.cc
               scheduler.cc ../inc/scheduler.h)

target_include_directories(${PROJECT_NAME}
                           PUBLIC
                           ${PROJECT_SOURCE_DIR}/inc)


target_link_libraries(${PROJECT_NAME} pthread)