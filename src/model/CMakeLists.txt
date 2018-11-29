file(GLOB_RECURSE SOURCES *.cpp)
add_library(model ${SOURCES})
target_include_directories(model PUBLIC .)
target_link_libraries(model core jsoncpp_lib_static)
