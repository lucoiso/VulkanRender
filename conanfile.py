# Copyright Notices: [...]

from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMakeToolchain


class VulkanRendererRecipe(ConanFile):
    name = "vulkan-renderer"
    version = "0.0.1"

    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"

    def requirements(self):
        # https://conan.io/center/recipes/glfw
        self.requires("glfw/3.4")

        # https://conan.io/center/recipes/imgui
        self.requires("imgui/1.90.5-docking")

        # https://conan.io/center/recipes/boost
        self.requires("boost/1.84.0")

        # https://conan.io/center/recipes/tinygltf
        self.requires("tinygltf/2.8.19")

        # https://conan.io/center/recipes/benchmark
        self.requires("benchmark/1.8.3")

        # https://conan.io/center/recipes/catch2
        self.requires("catch2/3.5.4")

    def configure(self):
        self.options["glfw/*"].shared = False
        self.options["imgui/*"].shared = True
        self.options["boost/*"].shared = True
        self.options["boost/*"].without_cobalt = True
        self.options["benchmark/*"].shared = True
        self.options["catch2/*"].shared = True
        self.options["catch2/*"].enable_exceptions = True

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.29]")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self, generator="Ninja")
        tc.generate()
