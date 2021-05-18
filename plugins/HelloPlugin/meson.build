dependencies = [inquisitor_api_dep]
 
sources = files([
    'HelloPlugin.cpp'
])
 
headers = files([
    'HelloPlugin.hpp'
])
 
hello_plugin = shared_library(
    'helloplugin',
    sources,
    extra_files: headers,
    dependencies: dependencies
)
