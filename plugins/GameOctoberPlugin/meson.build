dependencies = [stormkit_core_dep, stormkit_log_dep, json_dep, inquisitor_api_dep, dpp_dep, threads_dep]

sources = files([
    'GameOctoberPlugin.cpp'
])
 
headers = files([
    'GameOctoberPlugin.hpp',
    'Log.hpp'
])
 
hello_plugin = shared_library(
    'gameoctoberplugin',
    sources,
    extra_files: headers,
    dependencies: dependencies
)
