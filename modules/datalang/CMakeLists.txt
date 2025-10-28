project(datalang)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GenerateExportHeader)

set(SOURCES
    src/Lexer.cpp
    src/Parser.cpp
    src/ASTInterpreter.cpp
)


add_library(${PROJECT_NAME} SHARED ${SOURCES} )

target_compile_definitions(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_EXPORTS)

generate_export_header(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include 
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
      ${CMAKE_CURRENT_BINARY_DIR}
      )