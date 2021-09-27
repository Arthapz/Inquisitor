dependencies = [stormkit_core_dep, stormkit_log_dep, json_dep, inquisitor_api_dep, dpp_dep]

sources = files([
    'RandomQuotePlugin.cpp'
])
 
headers = files([
    'RandomQuotePlugin.hpp',
    'Log.hpp'
])
 
hello_plugin = shared_library(
    'randomquoteplugin',
    sources,
    extra_files: headers,
    dependencies: dependencies
)
