cmake_minimum_required(VERSION 3.23)
set(CROSS_COMPILE "lm32-elf-")
project(datamaster-firmware CXX ASM)

find_package(PkgConfig)
pkg_check_modules(lm32include REQUIRED lm32-include)
pkg_check_modules(lm32wrpcsw REQUIRED lm32-wrpc-sw)
pkg_check_modules(timingregisters REQUIRED timing-registers)

set(TARGET "ftm")
set(DEVICE "5AGXMA3D4")
set(FLASH "EPCQ256")
set(SPI_LANES "ASx4")
set(DM_VERSION "9.0.0")
set(DM_RELEASE "Fallout")
set(DEBUGLVL "0")

set(BUILDID_OFFS "0x100")
set(BUILDID_SIZE "0x400")

set(CBR_PF "" CACHE STRING "Platform")
set(TIMEZONE "CEST" CACHE STRING "Timezone")
# timezone can be set via %Z in cmake version 3.26
string(TIMESTAMP CBR_DATE "%a %b %d %H:%M:%S ${TIMEZONE} %Y")
message(STATUS "Build date: ${CBR_DATE}")
set(CBR_USR "" CACHE STRING "Prepared by (user)")
set(CBR_MAIL "" CACHE STRING "Prepared by (email)")
set(CBR_HOST "" CACHE STRING "Build host")
set(CBR_OS "" CACHE STRING "Operating system")
set(CBR_KRNL "" CACHE STRING "Kernel version")
set(CBR_GCC ${CMAKE_C_COMPILER_VERSION} CACHE STRING "GCC version")
set(CONFIG_RAMADDR "0x10000000" CACHE STRING "RAM address offset")
set(INT_BASE_ADR ${CONFIG_RAMADDR} CACHE STRING "INT base address")
set(SHARED_START "" CACHE STRING "Shared memory offset")
set(SHARED_SIZE "" CACHE STRING "Shared memory size")
set(THR_QTY "32" CACHE STRING "Thread quantity" )
set(GIT_LOG "" CACHE STRING "Git log")
set(GIT_BRANCH "" CACHE STRING "Git branch")
set(required_options
    CBR_USR
    CBR_MAIL
    CBR_HOST
    CBR_OS
    CBR_KRNL
    GIT_BRANCH)

foreach(option ${required_options})
    if (NOT DEFINED ${option} OR ${option} STREQUAL "")
        message(FATAL_ERROR "Option ${option} is required but not set.")
    endif()
endforeach()

set(USRCPUCLK "125000" CACHE STRING "User CPU Clock")
set(RAM_SIZE "131072")
set(SHARED_SIZE "0")

# RAM_SIZE below provided by project Makefile
set(CONFIG_RAMSIZE "${RAM_SIZE}")


# Warning: these 0x100 and 0x400 are separately defined in some x86 makefile.
set(CONFIG_BOOTSIZE "0x100")
set(CONFIG_BUILDIDSIZE "0x400")

# SHARED_SIZE below is optionally provided by project Makefile
set(CONFIG_SHAREDSIZE "98304")

# stack size is used to check for stack overflows, project Makefile can override it
set(CONFIG_STACKSIZE "10240")
set(CMAKE_C_FLAGS "-mmultiply-enabled -mbarrel-shift-enabled -Os -DUSRCPUCLK=${USRCPUCLK} -std=gnu99 -DCONFIG_WR_NODE -DCONFIG_PRINT_BUFSIZE=128 -DCONFIG_PRINTF_64BIT -DSDBFS_BIG_ENDIAN -ffunction-sections -fdata-sections -Wl,--gc-sections")


set(SHARED_SIZE "98304")
set(CMAKE_CXX_EXTENSIONS OFF)

configure_file(
    ${PROJECT_SOURCE_DIR}/src/ram.ld.S
    ${PROJECT_BINARY_DIR}/ram.ld
)

configure_file(
    ${PROJECT_SOURCE_DIR}/src/shared_mmap.h.S
    ${PROJECT_BINARY_DIR}/ftm_shared_mmap.h
)

configure_file(
    ${PROJECT_SOURCE_DIR}/src/config.h.in
    ${PROJECT_BINARY_DIR}/config.h
)

configure_file(
    ${PROJECT_SOURCE_DIR}/src/buildid.c.S
    ${PROJECT_BINARY_DIR}/buildid.c
    @ONLY
)

set(SOURCES
    ${PROJECT_SOURCE_DIR}/src/dm.c
    ${PROJECT_SOURCE_DIR}/src/main.c
    ${PROJECT_SOURCE_DIR}/src/stubs.c
    ${PROJECT_SOURCE_DIR}/src/crt0.S

    ${PROJECT_BINARY_DIR}/buildid.c
)

set(LM32_INCLUDE_SOURCES
    ${lm32include_INCLUDE_DIRS}/dbg.c
    ${lm32include_INCLUDE_DIRS}/aux.c
    ${lm32include_INCLUDE_DIRS}/irq.c
    ${lm32include_INCLUDE_DIRS}/mini_sdb.c

    ${lm32include_INCLUDE_DIRS}/sdb_add.c
	${lm32include_INCLUDE_DIRS}/assert.c
    ${lm32include_INCLUDE_DIRS}/stack-check.c
    ${lm32include_INCLUDE_DIRS}/ebm.c

    ${lm32wrpcsw_INCLUDE_DIRS}/dev/uart.c
    ${lm32wrpcsw_INCLUDE_DIRS}/lib/usleep.c
    ${lm32wrpcsw_INCLUDE_DIRS}/dev/devicelist.c
    ${lm32wrpcsw_INCLUDE_DIRS}/dev/syscon.c
    ${lm32wrpcsw_INCLUDE_DIRS}/pp_printf/printf.c
    ${lm32wrpcsw_INCLUDE_DIRS}/sdb-lib/glue.c
    ${lm32wrpcsw_INCLUDE_DIRS}/pp_printf/vsprintf-full.c
    ${lm32wrpcsw_INCLUDE_DIRS}/pp_printf/div64.c
)

add_executable(${PROJECT_NAME} ${SOURCES} ${LM32_INCLUDE_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ftm")
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".elf")

set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 99)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${lm32include_INCLUDE_DIRS}
    ${lm32wrpcsw_INCLUDE_DIRS}
    ${lm32wrpcsw_INCLUDE_DIRS}/pp_printf
    ${lm32wrpcsw_INCLUDE_DIRS}/include
    ${lm32wrpcsw_INCLUDE_DIRS}/sdb-lib
    ${timingregisters_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE datamaster-common_INTERFACE
    gcc
    c
)


# set linker flags for project
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib -T ram.ld")

target_compile_options(${PROJECT_NAME} PRIVATE -DDIAGNOSTICS -DCONFIG_ASSERT )

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET}.bin
)