project(scheduleCompare)
cmake_minimum_required(VERSION 3.21)

find_package(Boost REQUIRED COMPONENTS serialization graph regex)

add_executable(replaceChain
    ${PROJECT_SOURCE_DIR}/src/scheduleCompact.cpp
    ${PROJECT_SOURCE_DIR}/src/ScheduleGraph.cpp
    ${PROJECT_SOURCE_DIR}/src/replaceChainImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/parseSchedule.cpp
    ${PROJECT_SOURCE_DIR}/src/scheduleCompare.cpp
    ${PROJECT_SOURCE_DIR}/src/ScheduleVertex.cpp
    ${PROJECT_SOURCE_DIR}/src/ScheduleEdge.cpp
    ${PROJECT_SOURCE_DIR}/src/printSchedule.cpp
    ${PROJECT_SOURCE_DIR}/src/scheduleIsomorphism.cpp
)

add_executable(scheduleCompare
    ${PROJECT_SOURCE_DIR}/src/ScheduleGraph.cpp
    ${PROJECT_SOURCE_DIR}/src/parseSchedule.cpp
    ${PROJECT_SOURCE_DIR}/src/scheduleCompare.cpp
    ${PROJECT_SOURCE_DIR}/src/ScheduleVertex.cpp
    ${PROJECT_SOURCE_DIR}/src/ScheduleEdge.cpp
    ${PROJECT_SOURCE_DIR}/src/printSchedule.cpp
    ${PROJECT_SOURCE_DIR}/src/scheduleIsomorphism.cpp
)

target_include_directories(replaceChain
    PRIVATE ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(replaceChain
    PRIVATE Boost::serialization Boost::graph Boost::regex
)

target_include_directories(scheduleCompare
    PRIVATE ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(scheduleCompare
    PRIVATE Boost::serialization Boost::graph Boost::regex
)