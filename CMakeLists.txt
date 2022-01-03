cmake_minimum_required(VERSION 3.15)

project(ImGradientHDR)

include(ExternalProject)

set(imgui_source_path ${CMAKE_CURRENT_BINARY_DIR}/external/imgui/src/external_imgui/)
set(imgui_cmake_file_path ${imgui_source_path}/CMakeLists.txt)
message(${imgui_cmake_file_path})

ExternalProject_Add(
    external_imgui
    PREFIX  ${CMAKE_CURRENT_BINARY_DIR}/external/imgui
    DOWNLOAD_DIR    ${CMAKE_CURRENT_BINARY_DIR}/external/imgui
    URL https://github.com/ocornut/imgui/archive/refs/heads/master.zip
    PATCH_COMMAND   ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/CMakeList_imgui.txt ${imgui_cmake_file_path}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}
               -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
               -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
               -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
)

add_executable(test test/main.cpp)

add_dependencies(test external_imgui)
