file(GLOB_RECURSE SOURCES *.cpp)
add_library(model ${SOURCES})
target_link_libraries(model core)
