project(datalang)

include(GenerateExportHeader)

set(SOURCES
    src/Lexer.cpp
)


add_library(${PROJECT_NAME} SHARED ${SOURCES} )

target_compile_definitions(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_EXPORTS)

generate_export_header(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include 
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
      ${CMAKE_CURRENT_BINARY_DIR}
      )