dependencies = [stormkit_core_dep, stormkit_log_dep, json_dep, fmt_dep, inquisitor_api_dep, dpp_dep]

sources = files([
    'MelonPlugin.cpp'
])
 
headers = files([
    'MelonPlugin.hpp',
    'Log.hpp'
])
 
hello_plugin = shared_library(
    'melonplugin',
    sources,
    extra_files: headers,
    dependencies: dependencies
)
