from conan import ConanFile
from conan.tools.cmake import cmake_layout

class CrackHashConan(ConanFile):
    name = "crackhash-cpp"
    version = "1.0"
    
    settings = "os", "compiler", "build_type", "arch"
    
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        # nlohmann_json: Работа с JSON
        self.requires("nlohmann_json/3.11.2")
        
        # cpp-httplib: HTTP сервер и клиент
        self.requires("cpp-httplib/0.12.5")
        
        # openssl: Криптография (для вычисления MD5 хэшей)
        self.requires("openssl/3.1.3")

        # Логгирование
        self.requires("spdlog/1.17.0")

        self.requires("libuuid/1.0.3") 

        # XML парсинг (для XSD-моделей)
        self.requires("pugixml/1.15")

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.20]")

    def layout(self):
        cmake_layout(self)
