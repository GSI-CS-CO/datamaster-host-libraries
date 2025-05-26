project(datamaster-common)
include(GenerateExportHeader)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCE_DIR}/src/log.cpp)

generate_export_header(datamaster-common
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/datamaster_common_export.h
)

target_include_directories(
    ${PROJECT_NAME}
        PUBLIC ${PROJECT_SOURCE_DIR}/include
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(${PROJECT_NAME}_INTERFACE INTERFACE)
target_include_directories(
    ${PROJECT_NAME}_INTERFACE
        INTERFACE ${PROJECT_SOURCE_DIR}/include
)
