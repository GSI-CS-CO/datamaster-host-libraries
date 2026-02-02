project(datamaster_tests)

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
    ${PROJECT_SOURCE_DIR}/src/testValidation.cpp
    ${PROJECT_SOURCE_DIR}/src/graphHelper.cpp
    ${PROJECT_SOURCE_DIR}/src/testParser.cpp
    ${PROJECT_SOURCE_DIR}/src/testScheduleGraph.cpp
    ${PROJECT_SOURCE_DIR}/src/testTaskGraph.cpp
    ${PROJECT_SOURCE_DIR}/src/timingMessageIdMapping.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    GTest::gtest_main
    carpedm
    datamaster-common-lib
    Boost::serialization
    Boost::graph
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/modules/datamaster-host-libraries/src
    ${CMAKE_SOURCE_DIR}/test/stubs/
    ${PROJECT_SOURCE_DIR}/src
)

add_executable(LexerTest
    ${PROJECT_SOURCE_DIR}/src/datalang/LexerTest.cpp
    ${PROJECT_SOURCE_DIR}/src/datalang/ParserTest.cpp
    ${PROJECT_SOURCE_DIR}/src/datalang/ASTInterpreterTest.cpp
)

target_link_libraries(
    LexerTest
    GTest::gtest_main
    datalang
)

target_include_directories(
    LexerTest
    PRIVATE
    ${CMAKE_SOURCE_DIR}/modules/datamaster-host-libraries/src
    ${CMAKE_SOURCE_DIR}/test/stubs/
    ${PROJECT_SOURCE_DIR}/src
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
gtest_discover_tests(LexerTest)
