project(
    'Inquisitor',
    ['cpp'],
    version: '0.1.0',
    license: 'MIT',
    default_options: ['cpp_std=c++2a',
                      'stormkit:enable_entities=false',
                      'stormkit:enable_engine=false',
                      'stormkit:enable_window=false',
                      'stormkit:enable_window_xcb=false',
                      'stormkit:enable_window_wayland=false',
                      'stormkit:library_type=static',
                      'stormkit:buildtype=debug']
)

cmake = import('cmake')

stormkit_core_dep   = dependency('stormkit_core')
stormkit_log_dep    = dependency('stormkit_log')
stormkit_render_dep = dependency('stormkit_render')
stormkit_module_dep = dependency('stormkit_module')
json_dep            = dependency('nlohmann_json')
fmt_dep             = dependency('fmt')
ssl_dep             = dependency('OpenSSL')
boost_dep           = dependency('boost')
threads_dep           = dependency('threads')

discordpp_opt = cmake.subproject_options()
discordpp_opt.add_cmake_defines({'CMAKE_CXX_STANDARD': '17'})

if get_option('vcpkg_toolchain_file') != 'novcpkg'
    discordpp_opt.add_cmake_defines({'CMAKE_TOOLCHAIN_FILE': get_option('vcpkg_toolchain_file')})
endif

discordpp_proj = cmake.subproject('discordpp', options: discordpp_opt)
discordpp_plugin_overload_proj = cmake.subproject('discordpp-plugin-overload', options: discordpp_opt)
discordpp_reast_beast_proj = cmake.subproject('discordpp-reest-beast', options: discordpp_opt)
discordpp_websocket_simpleweb_proj = cmake.subproject('discordpp-websocket-simpleweb', options: discordpp_opt)
discordpp_plugin_ratelimit_proj = cmake.subproject('discord-plugin-ratelimit', options: discordpp_opt)
discordpp_plugin_responder_proj = cmake.subproject('discord-plugin-responder', options: discordpp_opt)

discordpp_dep  = discordpp_proj.dependency('discordpp')
discordpp_plugin_overload_dep = discordpp_plugin_overload_proj.dependency('discordpp-plugin-overload')
discordpp_reast_beast_dep = discordpp_reast_beast_proj.dependency('beast')
discordpp_websocket_simpleweb_dep = discordpp_websocket_simpleweb_proj.dependency('discordpp-websocket-simpleweb')
discordpp_plugin_ratelimit_dep = discordpp_plugin_ratelimit_proj.dependency('discordpp-plugin-ratelimit')
discordpp_plugin_responder_dep = discordpp_plugin_responder_proj.dependency('discordpp-plugin-responder')

dependencies = [stormkit_core_dep,
                stormkit_log_dep,
                stormkit_render_dep,
                stormkit_module_dep,
                json_dep,
                discordpp_dep,
                discordpp_plugin_overload_dep,
                discordpp_reast_beast_dep,
                discordpp_websocket_simpleweb_dep,
                discordpp_plugin_ratelimit_dep,
                discordpp_plugin_responder_dep,
                fmt_dep,
                ssl_dep,
                threads_dep]

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

subdir('plugins/')
