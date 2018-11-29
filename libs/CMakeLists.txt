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
IF (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libao/CMakeLists.txt")
    message(FATAL_ERROR "oops. Missing libao. had you cloned with --recursive?")
endif ()
IF (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/CMakeLists.txt")
    message(FATAL_ERROR "oops. Missing jsoncpp. had you cloned with --recursive?")
endif ()

# disable install directories
set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_CURRENT_BINARY_DIR}/install)
set(CMAKE_INSTALL_BINDIR ${CMAKE_CURRENT_BINARY_DIR}/install)
set(CMAKE_INSTALL_LIBDIR ${CMAKE_CURRENT_BINARY_DIR}/install)

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

set(JSONCPP_WITH_TESTS OFF CACHE BOOL "" FORCE)
set(JSONCPP_WITH_POST_BUILD_UNITTEST OFF CACHE BOOL "" FORCE)
set(JSONCPP_WITH_WARNING_AS_ERROR OFF CACHE BOOL "" FORCE)
set(JSONCPP_WITH_STRICT_ISO OFF CACHE BOOL "" FORCE)
set(JSONCPP_WITH_CMAKE_PACKAGE OFF CACHE BOOL "" FORCE)
set(JSONCPP_WITH_PKGCONFIG_SUPPORT OFF CACHE BOOL "" FORCE)
add_subdirectory(jsoncpp)

add_subdirectory(glad)
add_subdirectory(ffmpeg)
add_subdirectory(nativefiledialog)
add_subdirectory(libao)
