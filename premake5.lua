workspace "lava-renderer"

    location "."
    language "C++"

    architecture "x86_64"
    cppdialect "C++17"

    -- Arguments

    newoption {
        trigger     = "profile",
        value       = "BOOL",
        description = "Enable profiling dependencies"
    }

    -- Configurations

    configurations { "debug", "fast-compile", "profile", "release" }
    includedirs "include"

    if os.getenv("CXX") ~= nil and string.sub(os.getenv("CXX"), 1, 7)  == "clang++" then
        toolset "clang"
    end

    -- Warnings

    warnings "Extra"

    filter { "toolset:gcc" }
        buildoptions { "-fmax-errors=3" }

    filter { "toolset:clang"}
        buildoptions { "-ferror-limit=3" }

    -- Configurations

    filter { "configurations:debug" }
        symbols "on"
        optimize "off"
        -- vulkan.hpp was not ready for that
        -- buildoptions { "-Wsuggest-override", "-Wsuggest-final-types", "-Wsuggest-final-methods" }

    filter { "configurations:fast-compile" }
        symbols "off"
        optimize "debug"

    filter { "configurations:profile" }
        symbols "off"
        optimize "debug"

    filter { "configurations:release" }
        symbols "off"
        optimize "on"

        -- For release only, copy all the shared libs @fixme Doing that for each individual example...
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
