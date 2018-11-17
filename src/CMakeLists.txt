include(find_directories_containing)

find_directories_containing(HEADERS "*.h")
include_directories(${HEADERS})

add_subdirectory(core)
add_subdirectory(model)
add_subdirectory(editor)
add_subdirectory(showmaker)
