file(GLOB_RECURSE SOURCES *.cpp)
add_library(editor ${SOURCES})
target_include_directories(editor PUBLIC .)
target_link_libraries(editor model imgui)
