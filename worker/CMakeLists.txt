find_package(httplib REQUIRED)

file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)

file(GLOB_RECURSE HEADERS
    "include/*.hpp"
)

add_executable(worker ${HEADERS} ${SOURCES})

target_include_directories(worker PRIVATE
    include
    src
)

target_link_libraries(worker PRIVATE
    common_lib
    httplib::httplib
)

set_target_properties(worker PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
