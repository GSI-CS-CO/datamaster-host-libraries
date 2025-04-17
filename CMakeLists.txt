cmake_minimum_required(VERSION 3.21)

# Only set the cxx_standard if it is not set by someone else
if (NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
endif()

# strongly encouraged to enable this globally to avoid conflicts between
# -Wpedantic being enabled and -std=c++20 and -std=gnu++20 for example
# when compiling with PCH enabled
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig)
pkg_check_modules(etherbone REQUIRED etherbone)
pkg_check_modules(Boost REQUIRED)

project(carpedm
    VERSION 9.0.0
)

set(LZM_PATH ${PROJECT_SOURCE_DIR}/lib/lzma)

set(LZM_SOURCE_FILES
    ${LZM_PATH}/LzmaEnc.c
    ${LZM_PATH}/LzmaDec.c
    ${LZM_PATH}/LzFind.c
)

message("CMAKE_PROJECT_VERSION = ${CMAKE_PROJECT_VERSION}")
set(CMAKE_PROJECT_VERSION ${CMAKE_PROJECT_VERSION})
set(SOURCES
    ${PROJECT_SOURCE_DIR}/src/grouptable.cpp
    ${PROJECT_SOURCE_DIR}/src/reflocation.cpp
    ${PROJECT_SOURCE_DIR}/src/block.cpp
    ${PROJECT_SOURCE_DIR}/src/carpeDMsafe2remove.cpp
    ${PROJECT_SOURCE_DIR}/src/visitordownloadcrawler.cpp
    ${PROJECT_SOURCE_DIR}/src/idformat.cpp
    ${PROJECT_SOURCE_DIR}/src/lockmanager.cpp
    ${PROJECT_SOURCE_DIR}/src/carpeDMcommand.cpp
    ${PROJECT_SOURCE_DIR}/src/carpeDMuploadschedule.cpp
    ${PROJECT_SOURCE_DIR}/src/log.cpp
    # ${PROJECT_SOURCE_DIR}/src/ebsim.cpp
    ${PROJECT_SOURCE_DIR}/src/blocklock.cpp
    ${PROJECT_SOURCE_DIR}/src/lzmaCompression.cpp
    ${PROJECT_SOURCE_DIR}/src/carpeDMdownloadschedule.cpp
    ${PROJECT_SOURCE_DIR}/src/event.cpp
    ${PROJECT_SOURCE_DIR}/src/graph.cpp
    ${PROJECT_SOURCE_DIR}/src/globalreftable.cpp
    ${PROJECT_SOURCE_DIR}/src/validation.cpp
    ${PROJECT_SOURCE_DIR}/src/global.cpp
    ${PROJECT_SOURCE_DIR}/src/carpeDM.cpp
    ${PROJECT_SOURCE_DIR}/src/visitorvalidation.cpp
    ${PROJECT_SOURCE_DIR}/src/delayDiagnostics.cpp
    # ${PROJECT_SOURCE_DIR}/src/ebdev.cpp
    ${PROJECT_SOURCE_DIR}/src/meta.cpp
    ${PROJECT_SOURCE_DIR}/src/alloctable.cpp
    ${PROJECT_SOURCE_DIR}/src/carpeDMimpl.cpp
    ${PROJECT_SOURCE_DIR}/src/visitoruploadcrawler.cpp
    ${PROJECT_SOURCE_DIR}/src/visitorvertexwriter.cpp
    ${PROJECT_SOURCE_DIR}/src/dotstr.cpp
    ${PROJECT_SOURCE_DIR}/src/mempool.cpp
    ${PROJECT_SOURCE_DIR}/src/ebwrapper.cpp
    ${PROJECT_SOURCE_DIR}/src/hashmap.cpp
    ${PROJECT_SOURCE_DIR}/src/carpeDMdiagnostics.cpp
    ${PROJECT_SOURCE_DIR}/src/common.cpp
)

configure_file(${PROJECT_SOURCE_DIR}/src/target_info.h.in target_info.h)

add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
        ${LZM_PATH}
        ${PROJECT_SOURCE_DIR}/lib/dm_diag/
        "${CMAKE_CURRENT_BINARY_DIR}"
)

target_link_libraries(${PROJECT_NAME} etherbone)
target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE
    ETHERBONE_THROWS
    BUILDID_OFFS=0x100
    BUILDID_SIZE=0x400
)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)