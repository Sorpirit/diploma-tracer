# diploma-tracer

Instalation:

Dependancies:
* Vulkan API
* GLFW
* GLM
* Meson + Ninja
* Tracy

1. Download and isntall Vulkan SDK: https://vulkan.lunarg.com/sdk/home#windows
2. (Already in repo, can be skiped) Install pre-build 64 bit GLFW library: https://www.glfw.org/download.html
3. (Already in repo, can be skiped) Install GLM library https://github.com/g-truc/glm/releases
4. Installing Meson with pip(follow instructions here https://mesonbuild.com/Getting-meson.html)

Learing resources:
https://vulkan-tutorial.com/Development_environment
https://mesonbuild.com/Tutorial.html 

# Building
1. meson setup Build --buildtype=release --backend vs2022
2. meson compile -C Build