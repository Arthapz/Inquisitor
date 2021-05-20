dependencies = [stormkit_core_dep, stormkit_log_dep, json_dep, inquisitor_api_dep]

sources = files([
    'QuoteMessagePlugin.cpp'
])
 
headers = files([
    'QuoteMessagePlugin.hpp',
    'Log.hpp'
])
 
hello_plugin = shared_library(
    'quotemessageplugin',
    sources,
    extra_files: headers,
    dependencies: dependencies
)
