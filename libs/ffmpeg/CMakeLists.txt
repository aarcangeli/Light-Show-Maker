include(FetchContent)

add_library(FFmpeg INTERFACE)

set(FFMPEG_RUNTIME_LIBRARY "")
set(FFMPEG_FOUND OFF)

# On win64 use precompiled libraries
if (WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)

    macro(extract name archive path)
        set(${name} "${CMAKE_BINARY_DIR}/${path}")
        if (NOT ${name}_EXTRACTED)
            message(STATUS "extracting ${archive} into ${${name}}")
            execute_process(
                    COMMAND ${CMAKE_COMMAND} -E tar xzf "${archive}"
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            )
            set(${name}_EXTRACTED true CACHE BOOL "" FORCE)
        endif ()
    endmacro()

    extract(FFMPEG_DEV "${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg-4.1-win64-dev.zip" "ffmpeg-4.1-win64-dev")
    extract(FFMPEG_SHARED "${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg-4.1-win64-shared.zip" "ffmpeg-4.1-win64-shared")

    message(STATUS "FFMPEG_DEV: ${FFMPEG_DEV}")
    message(STATUS "FFMPEG_SHARED: ${FFMPEG_SHARED}")
    file(GLOB SOURCES ${FFMPEG_DEV}/lib/*.lib)
    target_include_directories(FFmpeg INTERFACE ${FFMPEG_DEV}/include)
    target_link_libraries(FFmpeg INTERFACE ${SOURCES})
    file(GLOB FFMPEG_RUNTIME_LIBRARY ${FFMPEG_SHARED}/bin/*.dll)
    set(FFMPEG_FOUND ON)

endif ()

# standard way
if (NOT FFMPEG_FOUND)
    find_package(FFmpeg)
    if (FFMPEG_FOUND)
        target_include_directories(FFmpeg INTERFACE ${FFMPEG_INCLUDE_DIR})
        target_link_libraries(FFmpeg INTERFACE ${FFMPEG_LIBRARIES})
    endif ()
endif ()

if (NOT FFMPEG_FOUND)
    message(FATAL_ERROR "ffmpeg not found")
endif ()

# global parameters
set(FFMPEG_RUNTIME_LIBRARY "${FFMPEG_RUNTIME_LIBRARY}" CACHE INTERNAL "" FORCE)

macro(ffmpeg_install_runtimes target)
    if (FFMPEG_RUNTIME_LIBRARY)
        add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FFMPEG_RUNTIME_LIBRARY}
                $<TARGET_FILE_DIR:${target}>)
        install(FILES ${FFMPEG_RUNTIME_LIBRARY} DESTINATION ${LSM_OUTPUT_DIRECTORY})
    endif ()
endmacro()
