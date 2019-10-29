cmake_minimum_required (VERSION 3.5)

set (PROJECT_NAME phase)

project (${PROJECT_NAME})

set (CMAKE_BUILD_TYPE Debug)
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Define sources
file (GLOB LIB_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)
file (GLOB DEMO_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/demo/*.cc)

# Add dependencies
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/deps/yaml-cpp)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/deps/clipp)

# Include directories
include_directories (
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CLIPP_INCLUDE_DIRS}
  ${YAML_CPP_INCLUDE_DIRS}
  )

# Link directories
link_directories (
  ${YAML_CPP_LIBRARY_DIRS}
  $ENV{CONDA_PREFIX}/lib
  )

add_library (${PROJECT_NAME} SHARED ${LIB_SOURCE})

target_link_libraries (
  ${PROJECT_NAME}
  yaml-cpp
  )

add_dependencies (
  ${PROJECT_NAME}
  ${CLIPP_EXTERNAL_PROJECT}
  ${YAML_CPP_EXTERNAL_PROJECT}
  )

add_executable (demo ${DEMO_SOURCE})
target_link_libraries(
  demo
  yaml-cpp
  ${PROJECT_NAME}
  )
