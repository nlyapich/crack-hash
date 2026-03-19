add_library(common_lib INTERFACE)

target_include_directories(common_lib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(libuuid REQUIRED)
find_package(pugixml REQUIRED)

target_link_libraries(common_lib INTERFACE
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    OpenSSL::Crypto
    libuuid::libuuid
    pugixml::pugixml
)

target_compile_features(common_lib INTERFACE cxx_std_17)
