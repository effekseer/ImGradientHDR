cmake_minimum_required(VERSION 3.15)

project(ImGradientHDR LANGUAGES CXX)

set(IMGRADIENTHDR_STANDALONE OFF)
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(IMGRADIENTHDR_STANDALONE ON)
endif()

option(IMGRADIENTHDR_BUILD_EXAMPLE "Build the GLFW/OpenGL example" ${IMGRADIENTHDR_STANDALONE})
option(IMGRADIENTHDR_BUILD_TESTS "Build automated tests" ${IMGRADIENTHDR_STANDALONE})

include(FetchContent)

if(NOT TARGET imgui)
    FetchContent_Declare(imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.91.9b
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(imgui)

    add_library(imgui STATIC
        "${imgui_SOURCE_DIR}/imgui.cpp"
        "${imgui_SOURCE_DIR}/imgui_draw.cpp"
        "${imgui_SOURCE_DIR}/imgui_tables.cpp"
        "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
    )
    target_include_directories(imgui PUBLIC "${imgui_SOURCE_DIR}")
    target_compile_features(imgui PUBLIC cxx_std_11)
endif()

add_library(ImGradientHDR STATIC src/ImGradientHDR.cpp)
add_library(ImGradientHDR::ImGradientHDR ALIAS ImGradientHDR)
target_include_directories(ImGradientHDR PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_features(ImGradientHDR PUBLIC cxx_std_14)
target_link_libraries(ImGradientHDR PRIVATE imgui)

if(MSVC)
    target_compile_options(ImGradientHDR PRIVATE /W4 /wd4100)
else()
    target_compile_options(ImGradientHDR PRIVATE -Wall -Wextra)
endif()

if(IMGRADIENTHDR_BUILD_EXAMPLE)
    find_package(OpenGL REQUIRED)
    if(NOT TARGET glfw)
        set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
        set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.4
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(glfw)
    endif()

    # An existing imgui target must also provide its source directory for the backends.
    # 既存のimguiターゲットを使用する場合、Backend用のソースディレクトリも指定する。
    if(NOT EXISTS "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp")
        message(FATAL_ERROR "Set imgui_SOURCE_DIR to the Dear ImGui source directory to build the example.")
    endif()
    add_executable(ImGradientHDRExample
        test/main.cpp
        "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp"
    )
    target_include_directories(ImGradientHDRExample PRIVATE "${imgui_SOURCE_DIR}/backends")
    target_link_libraries(ImGradientHDRExample PRIVATE
        ImGradientHDR imgui glfw OpenGL::GL ${CMAKE_DL_LIBS}
    )
endif()

if(IMGRADIENTHDR_BUILD_TESTS)
    enable_testing()
    add_executable(ImGradientHDRTests test/gradient_tests.cpp)
    target_link_libraries(ImGradientHDRTests PRIVATE ImGradientHDR)
    add_test(NAME ImGradientHDR.State COMMAND ImGradientHDRTests)
endif()
