dependencies = [stormkit_core_dep, stormkit_log_dep, json_dep, inquisitor_api_dep]

sources = files([
    'CleanChannelPlugin.cpp'
])
 
headers = files([
    'CleanChannelPlugin.hpp',
    'Log.hpp'
])
 
hello_plugin = shared_library(
    'cleanchannelplugin',
    sources,
    extra_files: headers,
    dependencies: dependencies
)
