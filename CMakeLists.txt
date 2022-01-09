cmake_minimum_required(VERSION 3.15)

project(ImGradientHDR)

include(ExternalProject)

find_package(OpenGL REQUIRED)

set(imgui_source_path ${CMAKE_CURRENT_BINARY_DIR}/external/imgui/src/external_imgui/)
set(imgui_cmake_file_path ${imgui_source_path}/CMakeLists.txt)
message(${imgui_cmake_file_path})

ExternalProject_Add(
    external_glfw3
    GIT_REPOSITORY https://github.com/glfw/glfw
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}
               -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
               -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
               -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
               -DGLFW_BUILD_EXAMPLES=OFF
               -DGLFW_BUILD_TESTS=OFF
               -DGLFW_BUILD_DOCS=OFF
               -DGLFW_INSTALL=ON
)

ExternalProject_Add(
    external_imgui
    DEPENDS external_glfw3
    PREFIX  ${CMAKE_CURRENT_BINARY_DIR}/external/imgui
    DOWNLOAD_DIR    ${CMAKE_CURRENT_BINARY_DIR}/external/imgui
    URL https://github.com/ocornut/imgui/archive/refs/heads/master.zip
    PATCH_COMMAND   ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists_imgui.txt ${imgui_cmake_file_path}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}
               -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
               -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
               -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
               -DGLFW_ROOT=${CMAKE_CURRENT_BINARY_DIR}/include
)

add_dependencies(external_imgui
    external_glfw3
)

add_executable(test test/main.cpp)

target_include_directories(test PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include ${OPENGL_INCLUDE_DIR})
target_link_directories(test PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/lib)

target_link_libraries(test PRIVATE glfw3 imgui ${OPENGL_LIBRARIES})

add_dependencies(test
    external_imgui
    external_glfw3
)
