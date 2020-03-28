cmake_minimum_required(VERSION 3.15)

project(
  sleip

  LANGUAGES
    CXX

  VERSION
    1.0.0
)

set(CMAKE_CXX_STANDARD "17" CACHE STRING "Required C++ standard")
if(CMAKE_CXX_STANDARD LESS 17)
  message(FATAL_ERROR "Minimum C++17 version not met!")
endif()

find_package(Boost 1.72 REQUIRED)

include(GNUInstallDirs)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
  ${PROJECT_NAME}
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}-${PROJECT_VERSION}>
)

target_link_libraries(${PROJECT_NAME} INTERFACE Boost::headers)

include(CTest)
if (BUILD_TESTING)
  find_package(Catch2 CONFIG REQUIRED)
  include(Catch)
  include("cmake/SleipAddTest.cmake")

  add_subdirectory(test)
endif()

option(
  SLEIP_ADD_SUBDIRECTORY
  "set(SLEIP_ADD_SUBDIRECTORY ON CACHE BOOL \"\") to disable installation rules allowing Sleip to support add_subdirectory"
  OFF
)

if (SLEIP_ADD_SUBDIRECTORY)
  add_library(Sleip::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
endif()

if (NOT SLEIP_ADD_SUBDIRECTORY)
  install(
    TARGETS
      ${PROJECT_NAME}

    EXPORT
      sleip-targets

    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )

  install(
    EXPORT
      sleip-targets

    NAMESPACE Sleip::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}-${PROJECT_VERSION}
  )

  install(
    DIRECTORY
      include/

    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}-${PROJECT_VERSION}
    FILES_MATCHING PATTERN "*.hpp"
  )

  include(CMakePackageConfigHelpers)
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/sleip-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
  )

  configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/sleip-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/sleip-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sleip-${PROJECT_VERSION}
  )

  install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/sleip-config.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/sleip-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sleip-${PROJECT_VERSION}
  )
endif()
