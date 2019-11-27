cmake_minimum_required (VERSION 3.5)

set (PROJECT_NAME phase)

project (${PROJECT_NAME})

set (CMAKE_BUILD_TYPE Debug)
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

option (BuildPintools "BuildPintools" ON)
option (BuildTools "BuildTools" ON)
option (BuildTests "BuildTests" OFF)

# Define sources
file (GLOB LIB_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)
file (GLOB DEMO_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/demo/*.cc)
file (GLOB TOOLS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/ref/tools/*.cc)

# Add dependencies
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/deps/yaml-cpp)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/deps/clipp)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/deps/pin)

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

# Macro for creating a subdirectory list
MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

# Build tracing libraries
SUBDIRLIST(PINTOOLS ${CMAKE_CURRENT_SOURCE_DIR}/ref/pintools)

if (BuildPintools)
  foreach (tool ${PINTOOLS})
    set (PINTOOL_OBJ_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/deps/pin/obj-intel64)
    file (GLOB TOOL_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/ref/pintools/${tool})
    file (MAKE_DIRECTORY ${PINTOOL_OBJ_DIRECTORY})
    file (MAKE_DIRECTORY ${TOOL_SOURCE}/obj-intel64)

    message(${TOOL_SOURCE})

    add_custom_target (
      ${tool}
      ALL
      COMMAND PIN_ROOT=${PIN_ROOT} PHASESIM_HOME=${CMAKE_CURRENT_SOURCE_DIR} GZIP_INCLUDES=${GZSTREAM_INCLUDE_DIRS} GZIP_LIBS=${GZSTREAM_LIBRARY_DIRS} make obj-intel64/${tool}.so
      WORKING_DIRECTORY  ${TOOL_SOURCE}
      DEPENDS always_rebuild ${PIN_EXTERNAL_PROJECT} ${GZSTREAM_EXTERNAL_PROJECT}
      )
  endforeach (tool)
endif (BuildPintools)

if (BuildTools)
  foreach (tool_path ${TOOLS_SOURCE})
    get_filename_component (tool_name ${tool_path} NAME_WE)

    add_executable (
      ${tool_name}
      ${tool_path}
      )

    target_link_libraries (
      ${tool_name}
      readline
      z
      yaml-cpp
      ${CMAKE_THREAD_LIBS_INIT}
      )
  endforeach (tool_path)
endif(BuildTools)

add_custom_command (
  OUTPUT always_rebuild
  COMMAND cmake -E echo
  )
