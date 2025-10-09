project(datamaster-tools)

set(TOOL_VER "0.39.1")
cmake_minimum_required(VERSION 3.21)

CPMAddPackage(
    NAME magic_enum
    GITHUB_REPOSITORY Neargye/magic_enum
    GIT_TAG v0.9.7
)

find_package(Boost COMPONENTS program_options serialization REQUIRED)

string(TIMESTAMP BUILD_DATE "%Y%m%d")

configure_file(
    ${PROJECT_SOURCE_DIR}/src/config.h.in
    config.h
    @ONLY
)

function(add_tool name)
    add_executable(${name} 
        ${PROJECT_SOURCE_DIR}/src/${name}.cpp
    )

    target_include_directories(
        ${name}
        PRIVATE
            ${PROJECT_BINARY_DIR}
            ${magic_enum_SOURCE_DIR}/include
    )

    target_link_libraries(${name} 
        PUBLIC datamaster-common
        carpedm
        magic_enum::magic_enum
       ${Boost_LIBRARIES}
    )
endfunction()

set(TOOLS dm-cmd dm-sched dm-sched2 dm-validate dm-validate-new dm-reduce md-test)

foreach(tool IN LISTS TOOLS)
    add_tool(${tool})
endforeach()

add_executable(pack-logs
    ${PROJECT_SOURCE_DIR}/src/pack-logs.cpp
)

install(
    TARGETS ${TOOLS} pack-logs
)