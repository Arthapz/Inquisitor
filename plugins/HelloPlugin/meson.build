dependencies = [stormkit_core_dep, stormkit_log_dep, json_dep, inquisitor_api_dep]

sources = files([
    'HelloPlugin.cpp'
])
 
headers = files([
    'HelloPlugin.hpp',
    'Log.hpp'
])
 
hello_plugin = shared_library(
    'helloplugin',
    sources,
    extra_files: headers,
    dependencies: dependencies
)
