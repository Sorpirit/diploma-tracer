
threads_dep = dependency('threads')

includes = [
    'public/tracy/TracyC.h',
    'public/tracy/Tracy.hpp',
    'public/tracy/TracyD3D11.hpp',
    'public/tracy/TracyD3D12.hpp',
    'public/tracy/TracyLua.hpp',
    'public/tracy/TracyOpenCL.hpp',
    'public/tracy/TracyOpenGL.hpp',
    'public/tracy/TracyVulkan.hpp'
]

client_includes = [
    'public/client/tracy_concurrentqueue.h',
    'public/client/tracy_rpmalloc.hpp',
    'public/client/tracy_SPSCQueue.h',
    'public/client/TracyArmCpuTable.hpp',
    'public/client/TracyCallstack.h',
    'public/client/TracyCallstack.hpp',
    'public/client/TracyDebug.hpp',
    'public/client/TracyDxt1.hpp',
    'public/client/TracyFastVector.hpp',
    'public/client/TracyLock.hpp',
    'public/client/TracyProfiler.hpp',
    'public/client/TracyRingBuffer.hpp',
    'public/client/TracyScoped.hpp',
    'public/client/TracyStringHelpers.hpp',
    'public/client/TracySysTime.hpp',
    'public/client/TracySysTrace.hpp',
    'public/client/TracyThread.hpp'
]

common_includes = [
    'public/common/tracy_lz4.hpp',
    'public/common/tracy_lz4hc.hpp',
    'public/common/TracyAlign.hpp',
    'public/common/TracyAlign.hpp',
    'public/common/TracyAlloc.hpp',
    'public/common/TracyApi.h',
    'public/common/TracyColor.hpp',
    'public/common/TracyForceInline.hpp',
    'public/common/TracyMutex.hpp',
    'public/common/TracyProtocol.hpp',
    'public/common/TracyQueue.hpp',
    'public/common/TracySocket.hpp',
    'public/common/TracyStackFrames.hpp',
    'public/common/TracySystem.hpp',
    'public/common/TracyUwp.hpp',
    'public/common/TracyYield.hpp'
]

tracy_header_files = common_includes + client_includes + includes

tracy_src = [
    'public/TracyClient.cpp'
]

tracy_public_include_dirs = include_directories('public')

compiler = meson.get_compiler('cpp')
override_options = []
if compiler.get_id() != 'msvc'
  override_options += 'cpp_std=c++11'
endif

tracy = static_library('tracy', tracy_src, tracy_header_files,
    dependencies        : [ threads_dep ],
    include_directories : tracy_public_include_dirs,
    override_options    : override_options)

tracy_dep = declare_dependency(
    link_with           : tracy,
    include_directories : tracy_public_include_dirs)

meson.override_dependency('tracy', tracy_dep)