tracer_utils_src = files(['TracerIO.cpp', 'Models\\TracerMesh.cpp' ])
tracer_utils_include = include_directories('.')

stb_lib_dir = '..\\..\\Libraries\\stb'
stb_include = include_directories(stb_lib_dir)


#Libraries\assimp-5.4.1\include
assimp_lib_dir = '..\\..\\Libraries\\assimp\\lib\\Release'
assimp_lib = cpp.find_library('assimp-vc143-mt',
               dirs : [meson.current_source_dir() + '\\' + assimp_lib_dir], required: true)
assimp_include = include_directories('..\\..\\Libraries\\assimp\\include')

tracer_utils = static_library('tracer_utils', tracer_utils_src, dependencies: [assimp_lib], include_directories : [tracer_utils_include, glm_include, stb_include, assimp_include])

tracer_utils_dep = declare_dependency(link_with : [tracer_utils], dependencies: [],
  include_directories : [tracer_utils_include])