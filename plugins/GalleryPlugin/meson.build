dependencies = [stormkit_core_dep, stormkit_log_dep, json_dep, inquisitor_api_dep, dpp_dep]

sources = files([
    'GalleryPlugin.cpp'
])
 
headers = files([
    'GalleryPlugin.hpp',
    'Log.hpp'
])
 
hello_plugin = shared_library(
    'galleryplugin',
    sources,
    extra_files: headers,
    dependencies: dependencies
)
