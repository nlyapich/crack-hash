file(GLOB_RECURSE COMMON_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(common_lib STATIC ${COMMON_SOURCES})

target_include_directories(common_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(libuuid REQUIRED)
find_package(pugixml REQUIRED)

target_link_libraries(common_lib PUBLIC
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    OpenSSL::Crypto
    libuuid::libuuid
    pugixml::pugixml
)

target_compile_features(common_lib PUBLIC cxx_std_17)
