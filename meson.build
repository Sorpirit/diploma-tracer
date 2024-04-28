
project('Tracer', ['c', 'cpp'],
  version: '0.0.1',
  default_options: ['cpp_std=c++17' ]
)

subdir('Source')
#Libraries
vulkan_lib_dir = 'C:\\VulkanSDK\\1.3.280.0'
glfw_lib_dir = 'Libraries\\glfw-3.4.bin.WIN64'

cpp = meson.get_compiler('cpp')
vulkan_lib = cpp.find_library('vulkan-1',
               dirs : [vulkan_lib_dir + '\\Lib'], required: true)

glfw_lib = cpp.find_library('glfw3',
               dirs : [meson.current_source_dir() + '\\' + glfw_lib_dir + '\\lib-vc2022'], required: true)
               

vulkan_include = include_directories(vulkan_lib_dir + '\\Include')
glfw_include = include_directories(glfw_lib_dir + '\\include')
glm_include = include_directories('Libraries\\glm-1.0.1')

trace_core = executable('Tracer', tracer_core_src,
  include_directories: [glfw_include, vulkan_include, glm_include],
  dependencies: [glfw_lib, vulkan_lib, tracer_utils_dep],
  install: true,
)

test('trace_core', trace_core)