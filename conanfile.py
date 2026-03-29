from conan import ConanFile
from conan.tools.cmake import cmake_layout

class EngineRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("vulkan-memory-allocator/3.3.0")
        self.requires("glfw/3.4")

    def layout(self):
        cmake_layout(self)
