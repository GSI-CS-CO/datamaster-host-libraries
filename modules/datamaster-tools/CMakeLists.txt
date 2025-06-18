project(datamaster-tools)

set(TOOL_VER "0.39.1")
cmake_minimum_required(VERSION 3.21)

find_package(Boost COMPONENTS program_options REQUIRED)

string(TIMESTAMP BUILD_DATE "%Y%m%d")

configure_file(
    ${PROJECT_SOURCE_DIR}/src/config.h.in
    config.h
    @ONLY
)

add_executable(dm-cmd 
    ${PROJECT_SOURCE_DIR}/src/dm-cmd.cpp
)

add_executable(dm-sched 
    ${PROJECT_SOURCE_DIR}/src/dm-sched.cpp
)

target_link_libraries(dm-cmd 
    PUBLIC datamaster-common
    carpedm
)

target_include_directories(
    dm-cmd
    PRIVATE
        ${PROJECT_BINARY_DIR}
)

target_include_directories(
    dm-sched
    PRIVATE
        ${PROJECT_BINARY_DIR}
)


target_link_libraries(dm-sched 
    PUBLIC datamaster-common
    carpedm
)

add_executable(dm-validate 
    ${PROJECT_SOURCE_DIR}/src/dm-validate.cpp
)

target_link_libraries(dm-validate 
    PUBLIC datamaster-common
    carpedm
   ${Boost_LIBRARIES}
)
target_include_directories(
    dm-validate
    PRIVATE
        ${PROJECT_BINARY_DIR}
)

install(
    TARGETS dm-cmd dm-sched dm-validate
)