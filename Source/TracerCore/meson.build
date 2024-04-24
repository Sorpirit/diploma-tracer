project('TracerCore', 'cpp'
  version: '0.1',
  default_options: ['cpp_std=c++17', 'warning_level=3'])

sources = files('main.cpp')

exe = executable('Tracer', sources, install: true)

test('basic', exe)
