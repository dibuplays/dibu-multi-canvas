cmake_minimum_required(VERSION 3.22...3.30)

option(DIBU_BUILD_PLUGIN "Build the native OBS plugin" ON)
option(DIBU_BUILD_TESTS "Build dependency-free core tests" ON)

if(NOT DIBU_BUILD_PLUGIN)
  project(dibu-multi-canvas VERSION 0.3.0 LANGUAGES CXX)
  add_library(dibu-layout-core STATIC src/link-model.cpp src/link-model.hpp)
  target_compile_features(dibu-layout-core PUBLIC cxx_std_17)
  target_include_directories(dibu-layout-core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
  if(DIBU_BUILD_TESTS)
    enable_testing()
    add_executable(dibu-core-tests tests/link-model-tests.cpp)
    target_compile_features(dibu-core-tests PRIVATE cxx_std_17)
    target_link_libraries(dibu-core-tests PRIVATE dibu-layout-core)
    add_test(NAME dibu-core-tests COMMAND dibu-core-tests)
  endif()
  return()
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)
project(${_name} VERSION ${_version} LANGUAGES CXX)

include(compilerconfig)
include(defaults)
include(helpers)

add_library(dibu-layout-core STATIC src/link-model.cpp src/link-model.hpp)
target_compile_features(dibu-layout-core PUBLIC cxx_std_17)
target_include_directories(dibu-layout-core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
set_target_properties(dibu-layout-core PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(${PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
find_package(obs-frontend-api REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_compile_definitions(${PROJECT_NAME} PRIVATE DIBU_PLUGIN_VERSION="${PROJECT_VERSION}")
target_link_libraries(${PROJECT_NAME} PRIVATE
  dibu-layout-core
  OBS::libobs
  OBS::obs-frontend-api
  Qt6::Core
  Qt6::Gui
  Qt6::Widgets
)

target_sources(${PROJECT_NAME} PRIVATE
  src/plugin-main.cpp
  src/canvas-service.cpp
  src/canvas-service.hpp
  src/dock-widget.cpp
  src/dock-widget.hpp
  src/output-service.cpp
  src/output-service.hpp
  src/preview-widget.cpp
  src/preview-widget.hpp
  src/settings-store.cpp
  src/settings-store.hpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  AUTOMOC ON
  AUTOUIC ON
  AUTORCC ON
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
)

if(OS_WINDOWS)
  configure_file(
    cmake/windows/resources/installer-Windows.iss.in
    "${CMAKE_CURRENT_BINARY_DIR}/installer-Windows.generated.iss"
    @ONLY
  )
endif()

set_target_properties_plugin(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})

if(DIBU_BUILD_TESTS)
  enable_testing()
  add_executable(dibu-core-tests tests/link-model-tests.cpp)
  target_compile_features(dibu-core-tests PRIVATE cxx_std_17)
  target_link_libraries(dibu-core-tests PRIVATE dibu-layout-core)
  add_test(NAME dibu-core-tests COMMAND dibu-core-tests)
endif()
