project(datamaster_tests)

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
    ${PROJECT_SOURCE_DIR}/src/timingMessageIdMapping.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    GTest::gtest_main
    carpedm
    Boost::serialization
    Boost::graph
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/modules/datamaster-host-libraries/src
)


include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
