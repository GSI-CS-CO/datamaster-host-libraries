project(datamaster-common-lib)

set(
    DATAMASTER_COMMON_SOURCES
        ${PROJECT_SOURCE_DIR}/src/ScheduleGraph.cpp
        ${PROJECT_SOURCE_DIR}/src/DatamasterEtherboneConnection.cpp
        ${PROJECT_SOURCE_DIR}/src/LM32MemoryRegion.cpp
        ${PROJECT_SOURCE_DIR}/src/DotParser.cpp
        ${PROJECT_SOURCE_DIR}/src/DatamasterFactory.cpp
        ${PROJECT_SOURCE_DIR}/src/ScheduleNodeMemoryManager.cpp
        ${PROJECT_SOURCE_DIR}/src/TaskGraph.cpp
)

CPMAddPackage(
    NAME magic_enum
    GITHUB_REPOSITORY Neargye/magic_enum
    GIT_TAG v0.9.7
)

# set C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(PkgConfig)
pkg_check_modules(etherbone REQUIRED etherbone)

include(GenerateExportHeader)

message("Using magic_enum from ${magic_enum_SOURCE_DIR} ${magic_enum_SOURCE_DIR}/include")

add_library(${PROJECT_NAME} STATIC ${DATAMASTER_COMMON_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include 
      ${magic_enum_SOURCE_DIR}/include
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
      ${CMAKE_CURRENT_BINARY_DIR}
      )

target_link_libraries(${PROJECT_NAME} PUBLIC magic_enum::magic_enum datamaster-common fmt spdlog::spdlog etherbone datalang)
target_compile_definitions(${PROJECT_NAME} PRIVATE DATAMASTER_COMMON_STATIC_DEFINE)

generate_export_header(${PROJECT_NAME})
