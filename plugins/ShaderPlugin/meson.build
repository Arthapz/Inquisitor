shaderc_dep = dependency('shaderc')

dependencies = [stormkit_core_dep, stormkit_log_dep, stormkit_render_dep, shaderc_dep, json_dep, inquisitor_api_dep]

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
