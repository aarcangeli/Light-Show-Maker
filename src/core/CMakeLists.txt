file(GLOB_RECURSE SOURCES *.cpp)
add_library(core ${SOURCES})
target_include_directories(core PUBLIC .)
target_link_libraries(core PUBLIC pathie)
