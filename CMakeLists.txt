cmake_minimum_required(VERSION 3.11)
project(MiniShell)

file(
  GLOB ${PROJECT_NAME}_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_SOURCES})

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/config)

if(TARGET OneMessage)
  target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE OneMessage)
  target_include_directories(
    ${PROJECT_NAME}
    PUBLIC $<TARGET_PROPERTY:OneMessage,INTERFACE_INCLUDE_DIRECTORIES>)
else()
  target_sources(${PROJECT_NAME}
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/om_core/om_list.c"
  )
  target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/om_core
  )
endif()
