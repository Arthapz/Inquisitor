shaderc_dep = dependency('shaderc')
#libwebm_dep = dependency('libwebm')
#libvpx_dep = dependency('unofficial-libvpx', method: 'cmake', modules: 'unofficial::libvpx::libvpx')
libavcodec_dep = dependency('libavcodec', fallback: ['FFmpeg', 'libavcodec_dep'])
libavformat_dep = dependency('libavformat', fallback: ['FFmpeg', 'libavformat_dep'])
libavutil_dep = dependency('libavutil', fallback: ['FFmpeg', 'libavutil_dep'])
swscale_dep = dependency('libswscale', fallback: ['FFmpeg', 'libswscale_dep'])

dependencies = [stormkit_core_dep, stormkit_log_dep, stormkit_render_dep, shaderc_dep, json_dep, inquisitor_api_dep, libavcodec_dep, libavformat_dep, libavutil_dep, swscale_dep]

sources = files([
    'ShaderPlugin.cpp'
])
 
headers = files([
    'ShaderPlugin.hpp',
    'Log.hpp'
])
 
hello_plugin = shared_library(
    'shaderplugin',
    sources,
    extra_files: headers,
    dependencies: dependencies
)
