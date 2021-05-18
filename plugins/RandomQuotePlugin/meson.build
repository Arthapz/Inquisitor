dependencies = [inquisitor_api_dep]
 
sources = files([
    'RandomQuotePlugin.cpp'
])
 
headers = files([
    'RandomQuotePlugin.hpp'
])
 
hello_plugin = shared_library(
    'randomquoteplugin',
    sources,
    extra_files: headers,
    dependencies: dependencies
)
