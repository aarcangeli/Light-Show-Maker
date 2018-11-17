set(GLFW_HOME "${LIBS_DIRECTORY}/glfw")
IF (NOT EXISTS "${GLFW_HOME}/CMakeLists.txt")
    message(FATAL_ERROR "oops. Missing imgui. had you cloned with --recursive?")
endif ()
set(GLFW_INCLUDE_DIR "${GLFW_HOME}/include")

set(ENKITS_BUILD_EXAMPLES OFF CACHE BOOL "Build basic example applications")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "GLFW lib only")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "GLFW lib only")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "GLFW lib only")
set(GLFW_BUILD_INSTALL OFF CACHE BOOL "GLFW lib only")
set(GLFW_INSTALL OFF CACHE BOOL "GLFW lib only")

add_subdirectory("${GLFW_HOME}" "${CMAKE_BINARY_DIR}/libs/glfw")
