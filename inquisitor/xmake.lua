target("inquisitor")
    set_languages("cxxlatest", "clatest")
    set_kind("binary")

    add_headerfiles("src/*.hpp")
    set_pcxxheader("src/CoreDependencies.hpp", {public = true})
    add_moduleorheaderfiles("src/*.mpp")
    add_files("src/*.cpp")

    add_deps("inquisitor_api")
    add_packages("libcurl")
