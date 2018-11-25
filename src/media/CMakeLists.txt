file(GLOB_RECURSE SOURCES *.cpp)
add_library(media ${SOURCES})
target_include_directories(media PUBLIC .)
target_link_libraries(media FFmpeg core)
