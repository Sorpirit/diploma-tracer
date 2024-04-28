tracer_utils_src = files(['TracerIO.cpp'])
tracer_utils_include = include_directories('.')

tracer_utils = static_library('tracer_utils', tracer_utils_src)
tracer_utils_dep = declare_dependency(link_with : tracer_utils,
  include_directories : tracer_utils_include)