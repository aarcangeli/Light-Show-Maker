include(ResourceCompiler)

file(GLOB_RECURSE SOURCES *.cpp)
compile_resources(SOURCES fonts fonts/*.ttf)
add_executable(showmaker ${SOURCES})
target_link_libraries(showmaker editor imgui nativefiledialog)
install(TARGETS showmaker RUNTIME DESTINATION ${OUTPUT_DIRECTORY})
set_target_properties(showmaker PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
