include(FetchContent)

add_library(FFmpeg INTERFACE)

set(FFMPEG_RUNTIME_LIBRARY "")
set(FFMPEG_RUNTIME_LIBRARY "")
set(FFMPEG_FOUND OFF)

# On win64 use precompiled libraries
if (WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)

    macro(download name url sha1)
        FetchContent_Declare(
                ${name}
                DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}
                URL ${url}
                URL_HASH SHA1=${sha1}
        )
        string(TOLOWER ${name} name_lower)

        message(STATUS "Downloading ${url}")
        FetchContent_Populate(${name})
        set(${name} ${${name_lower}_SOURCE_DIR})
        message(STATUS "${name} extracted into ${${name}}")

    endmacro()

    set(FFMPEG_URL https://FFmpeg.zeranoe.com/builds/win64/dev/FFmpeg-4.1-win64-dev.zip)
    set(FFMPEG_SHA1 8290EDEE18533DA26F57424E8FD78DE41FC81F5A)

    download(
            # NAME
            FFMPEG_DEV
            # URL
            https://FFmpeg.zeranoe.com/builds/win64/dev/FFmpeg-4.1-win64-dev.zip
            # SHA1
            8290EDEE18533DA26F57424E8FD78DE41FC81F5A
    )

    download(
            # NAME
            FFMPEG_SHARED
            # URL
            https://ffmpeg.zeranoe.com/builds/win64/shared/ffmpeg-4.1-win64-shared.zip
            # SHA1
            0AAC4FEFDAE4392ED670B16223687250993DB288
    )

    file(GLOB SOURCES ${FFMPEG_DEV}/lib/*.dll.a)
    target_include_directories(FFmpeg INTERFACE ${FFMPEG_DEV}/include)
    target_link_libraries(FFmpeg INTERFACE ${SOURCES})
    file(GLOB FFMPEG_RUNTIME_LIBRARY ${FFMPEG_SHARED}/bin/*.dll)
    set(FFMPEG_FOUND ON)

endif ()

# on other systems find if it is installed
if (NOT FFMPEG_FOUND)
    find_package(FFmpeg)
    if (FFMPEG_FOUND)
        target_include_directories(FFmpeg INTERFACE ${FFMPEG_INCLUDE_DIR})
        target_link_libraries(FFmpeg INTERFACE ${FFMPEG_LIBRARIES})
    endif ()
endif ()

# global parameters
set(FFMPEG_RUNTIME_LIBRARY "${FFMPEG_RUNTIME_LIBRARY}" CACHE INTERNAL "" FORCE)
set(FFMPEG_FOUND "${FFMPEG_FOUND}" CACHE INTERNAL "" FORCE)
if (FFMPEG_FOUND)
    target_compile_definitions(FFmpeg INTERFACE WITH_FFMPEG)
endif ()

macro(ffmpeg_install_runtimes target)
    if (FFMPEG_RUNTIME_LIBRARY)
        add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FFMPEG_RUNTIME_LIBRARY}
                $<TARGET_FILE_DIR:${target}>)
        install(FILES ${FFMPEG_RUNTIME_LIBRARY} DESTINATION ${OUTPUT_DIRECTORY})
    endif ()
endmacro()
