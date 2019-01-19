set(CMAKE_CXX_STANDARD 11)
set(ROOT_DIRECTORY ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${ROOT_DIRECTORY}/cmake")

IF (NOT LSM_VERSION)
    set(LSM_VERSION "Snapshot")
ENDIF ()

set(LSM_OUTPUT_DIRECTORY ${ROOT_DIRECTORY}/bin/lightshowmaker-v${LSM_VERSION})
set(CMAKE_INSTALL_PREFIX ${ROOT_DIRECTORY}/intermediates)

if (MINGW)
    # remove dependencies to libstdc++-6.dll, libgcc_s_dw2-1.dll, libwinpthread-1.dll
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
endif ()

if (MSVC)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /ENTRY:mainCRTStartup")
endif ()

message(STATUS "LSM_VERSION: ${LSM_VERSION}")
message(STATUS "LSM_OUTPUT_DIRECTORY: ${LSM_OUTPUT_DIRECTORY}")

add_definitions("-DLSM_VERSION=\"${LSM_VERSION}\"")
