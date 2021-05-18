project(
    'Inquisitor',
    ['cpp'],
    version: '0.1.0',
    license: 'MIT',
    default_options: ['cpp_std=c++20',
                      'stormkit:enable_entities=false',
                      'stormkit:enable_engine=false',
                      'stormkit:enable_window=false',
                      'stormkit:enable_window_xcb=false',
                      'stormkit:enable_window_wayland=false',
                      'stormkit:library_type=static']
)

cmake = import('cmake')

stormkit_core_dep   = dependency('stormkit_core')
stormkit_log_dep    = dependency('stormkit_log')
stormkit_render_dep = dependency('stormkit_render')
stormkit_module_dep = dependency('stormkit_module')
rapidjson_dep       = dependency('RapidJSON')
openssl_dep         = dependency('OpenSSL')
fmt_dep             = dependency('fmt')

sleepy_discord_opt = cmake.subproject_options()
sleepy_discord_opt.add_cmake_defines({'USE_ASIO': true})
sleepy_discord_opt.add_cmake_defines({'USE_WEBSOCKETPP': true})
sleepy_discord_opt.add_cmake_defines({'USE_UWEBSOCKETS': false})
sleepy_discord_opt.add_cmake_defines({'USE_CPR': true})
sleepy_discord_opt.add_cmake_defines({'USE_ZLIB': true})

sleepy_discord_proj = cmake.subproject('sleepy-discord', options: sleepy_discord_opt)

sleepy_discord_dep  = sleepy_discord_proj.dependency('sleepy-discord')
cpr_dep  = sleepy_discord_proj.dependency('cpr')

dependencies = [stormkit_core_dep, stormkit_log_dep, stormkit_render_dep, stormkit_module_dep, rapidjson_dep, sleepy_discord_dep, cpr_dep, openssl_dep, fmt_dep]

inquisitor_api_inc = include_directories(['api/'])

sources = files([
    'src/main.cpp',
    'src/Bot.cpp',
    'src/Inquisitor.cpp',
])

headers = files([
    'src/Bot.hpp',
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
