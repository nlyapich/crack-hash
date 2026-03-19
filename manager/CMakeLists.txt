find_package(httplib REQUIRED)
find_package(libuuid REQUIRED)

file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)

file(GLOB_RECURSE HEADERS
    "include/*.hpp"
)

add_executable(manager ${HEADERS} ${SOURCES})

target_include_directories(manager PRIVATE
    include
    src
)

target_link_libraries(manager PRIVATE
    common_lib
    httplib::httplib
    libuuid::libuuid
    pthread
)

set_target_properties(manager PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
