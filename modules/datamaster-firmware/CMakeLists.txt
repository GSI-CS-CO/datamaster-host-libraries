cmake_minimum_required(VERSION 3.21)
set(CROSS_COMPILE "lm32-elf-")
project(datamaster-firmware
    VERSION 9.0.0
)

find_package(PkgConfig)
pkg_check_modules(lm32include REQUIRED lm32-include)
pkg_check_modules(lm32ppprintf REQUIRED lm32-pp-printf)
message(STATUS "lm32include_INCLUDE_DIRS: ${lm32include_INCLUDE_DIRS}")
message(STATUS "lm32ppprintf_INCLUDE_DIRS: ${lm32ppprintf_INCLUDE_DIRS}")

if (NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
endif()

set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES
    ${PROJECT_SOURCE_DIR}/src/dm.c
    ${PROJECT_SOURCE_DIR}/src/main.c
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${lm32include_INCLUDE_DIRS}
    ${lm32ppprintf_INCLUDE_DIRS}
)