include(ResourceCompiler)

file(GLOB_RECURSE SOURCES *.cpp)
compile_resources(SOURCES fonts fonts/*.ttf)
add_executable(lightshowmaker ${SOURCES})
target_link_libraries(lightshowmaker editor imgui nativefiledialog)
install(TARGETS lightshowmaker RUNTIME DESTINATION ${LSM_OUTPUT_DIRECTORY})
target_include_directories(lightshowmaker PUBLIC .)
ffmpeg_install_runtimes(lightshowmaker)

if (MSVC)
    set_target_properties(lightshowmaker PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
endif ()
