target("inquisitor_api")
    set_kind("static")
    set_languages("cxxlatest", "clatest")

    add_headerfiles("include/(**.hpp)")
    set_pcxxheader("include/inquisitor/CoreDependencies.hpp", {public = true})
    add_moduleorheaderfiles("src/*.mpp", {dir = "include/inquisitor/"})
    add_files("src/*.cpp")

    add_includedirs("include", {public = true})

    add_packages("glm", "unordered_dense", "nlohmann_json", "frozen", "dpp", {public = true})
    add_packages("stormkit", {components = {"core", "log"}, public = true})
