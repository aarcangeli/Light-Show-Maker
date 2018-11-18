# verify submodules
IF (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/glfw/CMakeLists.txt")
    message(FATAL_ERROR "oops. Missing glfw. had you cloned with --recursive?")
endif ()
IF (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h")
    message(FATAL_ERROR "oops. Missing imgui. had you cloned with --recursive?")
endif ()
IF (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/nativefiledialog/CMakeLists.txt")
    message(FATAL_ERROR "oops. Missing nativefiledialog. had you cloned with --recursive?")
endif ()

# configure glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "GLFW lib only")
add_subdirectory(glfw)

# configure imgui
find_package(OpenGL REQUIRED)
set(
        IMGUI_SOURCES
        imgui/imgui.cpp
        imgui/imgui_widgets.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_demo.cpp
        imgui/examples/imgui_impl_glfw.cpp
        imgui/examples/imgui_impl_opengl3.cpp
)
add_library(imgui ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC imgui imgui/examples ${OPENGL_INCLUDE_DIR})
target_compile_definitions(imgui PRIVATE -DIMGUI_IMPL_OPENGL_LOADER_GLAD)
target_link_libraries(imgui glad glfw ${GLFW_LIBRARIES})

add_subdirectory(glad)
add_subdirectory(nativefiledialog)
