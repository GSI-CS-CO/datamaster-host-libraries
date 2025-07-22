cmake_minimum_required(VERSION 3.21)

project(datamaster
    VERSION 9.0.0
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BUILD_FIRMWARE OFF CACHE BOOL "Build firmware? If false, host libraries are built")



if(${BUILD_FIRMWARE})

  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out)

  add_subdirectory(modules/datamaster-common)
  add_subdirectory(modules/datamaster-firmware)

elseif(${BUILD_HOST})
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)

  message(STATUS "Build to ${PROJECT_SOURCE_DIR}/bin")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

  add_subdirectory(modules/datamaster-common)
  add_subdirectory(modules/datamaster-host-libraries)
  # no fesa here, if you want to pollute your beautiful system with that crap...HOW ABOUT NO
  add_subdirectory(modules/datamaster-tools)
  add_subdirectory(test/test_programs)


  if(ENABLE_TESTS)
    add_subdirectory(test/unit)
  endif()

  include(CPack)

else()
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
  add_compile_options(-g -O0 -fno-omit-frame-pointer)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out)

  add_subdirectory(modules/datamaster-common)
  add_subdirectory(modules/datamaster-host-libraries)
  add_subdirectory(modules/fesa-class-generator)
  add_subdirectory(modules/fesa-du-generator)
  add_subdirectory(modules/datamaster-tools)
  add_subdirectory(test/test_programs)
endif()
