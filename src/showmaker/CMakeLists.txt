include(glfw)
include(imgui)
include(glad)

find_package(OpenGL REQUIRED)

include_directories(
        ${IMGUI_INCLUDE_DIR}
        ${GLFW_INCLUDE_DIR}
        ${OPENGL_INCLUDE_DIR}
        ${GLAD_INCLUDE_DIR}
)

file(GLOB_RECURSE SOURCES *.cpp)
add_executable(showmaker WIN32 ${SOURCES})

target_link_libraries(
        showmaker
        core
        editor
        glfw ${GLFW_LIBRARIES}
        opengl32
        glad
)

install(TARGETS showmaker RUNTIME DESTINATION ${OUTPUT_DIRECTORY})
