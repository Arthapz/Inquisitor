dependencies = [inquisitor_api_dep]
 
sources = files([
    'QuoteMessagePlugin.cpp'
])
 
headers = files([
    'QuoteMessagePlugin.hpp'
])
 
hello_plugin = shared_library(
    'quotemessageplugin',
    sources,
    extra_files: headers,
    dependencies: dependencies
)
