file(GLOB_RECURSE SOURCES *.cpp)
add_executable(showmaker WIN32 ${SOURCES})
target_link_libraries(showmaker editor imgui)
install(TARGETS showmaker RUNTIME DESTINATION ${OUTPUT_DIRECTORY})
