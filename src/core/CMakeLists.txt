file(GLOB_RECURSE SOURCES *.cpp)
add_library(core ${SOURCES})
target_include_directories(core PUBLIC .)
