# Copyright Notices: [...]

import os
from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMakeToolchain
from conan.tools.files import copy


class VulkanRendererRecipe(ConanFile):
    name = "vulkan-renderer"
    version = "0.0.1"

    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"

    default_options = {"*:shared": True}

    def requirements(self):
        # https://conan.io/center/recipes/glfw
        self.requires("glfw/[>=3.4]")

        # https://conan.io/center/recipes/imgui
        self.requires("imgui/1.90.4-docking")

        # https://conan.io/center/recipes/boost
        self.requires("boost/[>=1.84]",
                      options={
                          "without_cobalt": True
                      })

        # https://conan.io/center/recipes/benchmark
        self.requires("benchmark/[>=1.8]")

        # https://conan.io/center/recipes/catch2
        self.requires("catch2/[>=3.5]")

        # https://conan.io/center/recipes/tinygltf
        self.requires("tinygltf/[>=2.8]")

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.28]")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self, generator="Ninja")
        tc.generate()

        for dep in self.dependencies.values():
            if dep.cpp_info.bindirs:
                copy(pattern="*.dll", dst=os.path.join(self.build_folder, str(self.settings.build_type), "bin"), src=dep.cpp_info.bindirs[0], conanfile=self)
                copy(pattern="*.dylib", dst=os.path.join(self.build_folder, str(self.settings.build_type), "bin"), src=dep.cpp_info.bindirs[0], conanfile=self)
                copy(pattern="*.so", dst=os.path.join(self.build_folder, str(self.settings.build_type), "bin"), src=dep.cpp_info.bindirs[0], conanfile=self)
