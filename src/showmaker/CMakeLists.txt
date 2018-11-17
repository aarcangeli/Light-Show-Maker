file(GLOB_RECURSE SOURCES *.cpp)
add_executable(showmaker ${SOURCES})

install(TARGETS showmaker RUNTIME DESTINATION ${OUTPUT_DIRECTORY})
