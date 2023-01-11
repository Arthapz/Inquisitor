local allowedmodes = {
    "debug",
    "release",
    "releasedbg",
    "asan",
    "lsan",
    "ubsan",
    "profile"
}

add_repositories("nazara-engine-repo https://github.com/NazaraEngine/xmake-repo")

set_xmakever("2.7.4")
set_project("Inquisitor")

set_version("4.0.0", { build = "%Y%m%d%H%M" } )

add_rules("plugin.vsxmake.autoupdate")
add_rules("mode.debug",
          "mode.release",
          "mode.releasedbg",
          "mode.asan",
          "mode.lsan",
          "mode.tsan",
          "mode.ubsan",
          "mode.profile")

if not is_plat("windows") then
    add_rules("mode.valgrind")
    table.append(allowedmodes, "valgrind")
end

set_allowedmodes(allowedmodes)
set_allowedplats("windows", "mingw", "linux", "macosx")
set_allowedarchs("windows|x64", "mingw|x86_64", "linux|x86_64", "macosx|x86_64")

set_fpmodels("fast")
set_optimize("fastest")
set_warnings("allextra", "error")
set_policy("build.optimization.lto", true)

add_vectorexts("mms", "neon", "avx", "avx2", "sse", "sse2", "sse3", "sse4")

if is_mode("release") then
   set_symbols("hidden")
else
   set_symbols("debug", "hidden")
end

if is_plat("windows") then
    set_runtimes(is_mode("debug") and "MDd" or "MD")
    add_defines("NOMINMAX", "WIN32_LEAN_AND_MEAN", { public = true })
    add_defines("_CRT_SECURE_NO_WARNINGS")
    add_cxxflags("/utf-8", "/bigobj", "/permissive-", "/Zc:wchar_t", "/Zc:__cplusplus", "/Zc:externConstexpr", "/Zc:inline", "/Zc:lambda", "/Zc:preprocessor", "/Zc:referenceBinding", "/Zc:strictStrings", "/Zc:throwingNew")
    add_cxflags("/wd4251") -- Disable warning: class needs to have dll-interface to be used by clients of class blah blah blah
    add_cxflags("/wd4297")
    add_cxflags("/wd5063")
    add_cxflags("/wd5260")
    add_cxflags("/wd5050")
    add_cxflags("/wd4005")
    add_cxflags("/wd4611") -- Disable setjmp warning
    add_ldflags("/INCREMENTAL:NO")
else
    add_cxxflags("-std=c++2b") -- for clangd
    add_syslinks("stdc++_libbacktrace")
    add_cxxflags("-Wno-missing-field-initializers")
end

add_moduleorheaderfiles = add_headerfiles
if has_config("use_modules") then
    add_moduleorheaderfiles = add_files
end

add_repositories("localrepo thirdparty")
add_requires("frozen", "unordered_dense", "glm", "libcurl", "dpp", "nlohmann_json", {debug = is_mode("debug")})

local stormkit_configs = {
    enable_log = true
}

add_requires("stormkit", {configs = {enable_log = true}, debug = is_mode("debug")})

option("use_modules")
    set_default(false)
    add_defines("STORMKIT_BUILD_MODULES")

option("use_cpp23_msvc_import")
    add_deps("use_modules")
    set_default(false)

option("enable_pch")
    set_default(true)

includes("api/xmake.lua")
includes("inquisitor/xmake.lua")
