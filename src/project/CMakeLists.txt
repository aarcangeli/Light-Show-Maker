file(GLOB_RECURSE SOURCES *.cpp)
add_library(project ${SOURCES})
target_link_libraries(project core)
