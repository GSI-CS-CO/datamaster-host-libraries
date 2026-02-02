project(datamaster_integration_tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

cmake_policy(SET CMP0135 NEW)
include(FetchContent)
FetchContent_Declare(
googletest
URL https://github.com/google/googletest/archive/52eb8108c5bdec04579160ae17225d66034bd723.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

find_package(Boost REQUIRED COMPONENTS serialization graph)

enable_testing()

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/DatalangInterpreterTests.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    GTest::gtest_main
    carpedm
    datamaster-common-lib
    Boost::serialization
    Boost::graph
    datalang
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/modules/datamaster-host-libraries/src
    ${CMAKE_SOURCE_DIR}/modules/datalang
    ${CMAKE_SOURCE_DIR}/test/stubs
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
