cmake_minimum_required(VERSION 3.21)


# Only set the cxx_standard if it is not set by someone else
if (NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

# strongly encouraged to enable this globally to avoid conflicts between
# -Wpedantic being enabled and -std=c++20 and -std=gnu++20 for example
# when compiling with PCH enabled
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(datamaster-host-build
    VERSION 9.0.0
)

add_subdirectory(modules/datamaster-host-libraries)
add_subdirectory(modules/fesa-class-generator)
add_subdirectory(modules/fesa-du-generator)
add_subdirectory(modules/datamaster-firmware)
