workspace "lava-renderer"

    location "build/gen"
    language "C++"

    architecture "x86_64"

    -- Configurations

    configurations { "debug", "release" }

    filter { "configurations:debug" }
        flags { "c++14" }
        symbols "on"

    filter { "configurations:release" }
        flags { "c++14" }
        optimize "on"

    filter {}

    -- Targets

    targetdir "build/%{cfg.longname}/"
    objdir "build/%{cfg.longname}/obj/"

    -- Includes

    include "premake"
    include "external"
    include "source"
