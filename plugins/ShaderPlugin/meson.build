shaderc_dep = dependency('shaderc')
libwebp_dep = dependency('libwebp')

dependencies = [stormkit_core_dep, stormkit_log_dep, stormkit_render_dep, shaderc_dep, json_dep, inquisitor_api_dep, libwebp_dep]

sources = files([
    'ShaderPlugin.cpp'
])
 
headers = files([
    'ShaderPlugin.hpp',
    'Log.hpp'
])
 
hello_plugin = shared_library(
    'shaderplugin',
    sources,
    extra_files: headers,
    dependencies: dependencies
)
