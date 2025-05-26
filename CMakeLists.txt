cmake_minimum_required(VERSION 3.21)

project(datamaster
    VERSION 9.0.0
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BUILD_FIRMWARE OFF CACHE BOOL "Build firmware? If false, host libraries are built")

# set( CMAKE_VERBOSE_MAKEFILE on )

# set output directory to ${PROJECT_SOURCE_DIR}/out
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out)

if(${BUILD_FIRMWARE})
  add_subdirectory(modules/datamaster-common)
  add_subdirectory(modules/datamaster-firmware)
else()
  set(CMAKE_CXX_STANDARD 14)

  add_subdirectory(modules/datamaster-common)
  add_subdirectory(modules/datamaster-host-libraries)
  add_subdirectory(modules/fesa-class-generator)
  add_subdirectory(modules/fesa-du-generator)

  install(
    TARGETS Generator_DU
  )

endif()