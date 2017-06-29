workspace "lava-renderer"

    location "."
    language "C++"

    architecture "x86_64"

    -- Configurations

    configurations { "debug", "release" }

    includedirs "include"

    filter { "configurations:debug" }
        flags { "c++14" }
        buildoptions { "-fmax-errors=3", "-Wall", "-Wextra" }
        -- vulkan.hpp was not ready for that
        -- buildoptions { "-Wsuggest-override", "-Wsuggest-final-types", "-Wsuggest-final-methods" }
        symbols "on"

    filter { "configurations:release" }
        flags { "c++14" }
        optimize "on"

        -- For release only, copy all the shared libs
        postbuildcommands  { "cp -v external/lib/*.so* build/release/" }

    filter {}

    -- Targets

    targetdir "build/%{cfg.longname}/"
    objdir "build/%{cfg.longname}/obj/"

    -- Includes

    include "premake"
    include "external"
    include "source"
    include "examples"
