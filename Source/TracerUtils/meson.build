tracer_utils_src = files(['TracerIO.cpp'])
tracer_utils_include = include_directories('.')

stb_lib_dir = '..\\..\\Libraries\\stb'
stb_include = include_directories(stb_lib_dir)

tracer_utils = static_library('tracer_utils', tracer_utils_src, include_directories : [tracer_utils_include, stb_include])
tracer_utils_dep = declare_dependency(link_with : tracer_utils,
  include_directories : [tracer_utils_include])