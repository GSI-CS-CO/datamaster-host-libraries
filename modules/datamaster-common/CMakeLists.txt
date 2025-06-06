project(datamaster-common)

set(DATAMASTER_COMMON_SOURCES ${PROJECT_SOURCE_DIR}/src/log.cpp CACHE INTERNAL "Datamaster common sources")

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR}/include)
