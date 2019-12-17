cmake_minimum_required(VERSION 3.16.0)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(VCPKG_TARGET_TRIPLET "x64-windows-static")
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

project(tessViewer)

add_subdirectory(tessViewer)
