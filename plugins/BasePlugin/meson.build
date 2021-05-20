dependencies = [stormkit_core_dep, stormkit_log_dep, json_dep, inquisitor_api_dep]

sources = files([
    'BasePlugin.cpp'
])
 
headers = files([
    'BasePlugin.hpp',
    'Log.hpp'
])
 
hello_plugin = shared_library(
    'baseplugin',
    sources,
    extra_files: headers,
    dependencies: dependencies
)
