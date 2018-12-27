set(CMAKE_CXX_STANDARD 11)
set(ROOT_DIRECTORY ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${ROOT_DIRECTORY}/cmake")
set(OUTPUT_DIRECTORY ${ROOT_DIRECTORY}/bin)

if (MINGW)
    # remove dependencies to libstdc++-6.dll, libgcc_s_dw2-1.dll, libwinpthread-1.dll
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
endif ()

if (MSVC)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /ENTRY:mainCRTStartup")
endif ()
