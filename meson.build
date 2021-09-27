project(
    'Inquisitor',
    ['cpp'],
    version: '0.1.0',
    license: 'MIT',
    default_options: [
                      'stormkit:enable_entities=false',
                      'stormkit:enable_engine=false',
                      'stormkit:enable_render=true',
                      'stormkit:enable_window=false',
                      'stormkit:enable_window_xcb=false',
                      'stormkit:enable_window_wayland=false',
                      'stormkit:library_type=static',
                      'stormkit:buildtype=debug']
)

compiler = meson.get_compiler('cpp')
build_type = get_option('buildtype')

if compiler.get_argument_syntax() == 'msvc'
    add_project_arguments(['/std:c++latest', '/permissive-', '/DNOMINMAX', '/DWIN32_LEAN_AND_MEAN'], language: ['cpp'])
else
    add_project_arguments(['-std=c++20'], language: ['cpp'])
endif


cmake = import('cmake')

stormkit_core_dep   = dependency('stormkit_core')
stormkit_log_dep    = dependency('stormkit_log')
if get_option('shader_plugin').enabled()
    stormkit_render_dep = dependency('stormkit_render')
endif
stormkit_module_dep = dependency('stormkit_module')
json_dep            = dependency('nlohmann_json')
fmt_dep             = dependency('fmt')
if compiler.get_argument_syntax() == 'msvc'
curl_dep            = dependency('CURL', method: 'cmake', modules: ['CURL::libcurl'])
else
curl_dep            = dependency('libcurl')
endif

threads_dep = dependency('threads')
dpp_dep = dependency('dpp')

dependencies = [stormkit_core_dep,
                stormkit_log_dep,
                stormkit_module_dep,
                json_dep,
                fmt_dep,
                curl_dep,
                threads_dep,
                dpp_dep]

if get_option('shader_plugin').enabled()
    dependencies += [stormkit_render_dep]
endif

inquisitor_api_inc = include_directories(['api/'])

sources = files([
    'src/main.cpp',
    'src/Inquisitor.cpp',
])

headers = files([
    'src/Fwd.hpp',
    'src/Inquisitor.hpp',
    'src/Log.hpp',
])

api_sources = files([
    'api/PluginInterface.cpp'
])

api_headers = files([
    'api/PluginInterface.hpp'
])


inquisitor_api = static_library(
    'inquisitor_api',
    api_sources,
    dependencies: json_dep,
    extra_files: api_headers
)

inquisitor_api_dep = declare_dependency(
    link_with: inquisitor_api,
    include_directories: inquisitor_api_inc,
)

inquisitor = executable(
    'inquisitor',
    sources,
    dependencies: dependencies + [inquisitor_api_dep],
    extra_files: headers
)

#subdir('plugins/')
