project "lava-magma"
    kind "SharedLib"
    pic "on"

    pchheader "magma/pch.hpp"
    files "magma/**"

    function magmaDependencies()
        if options.windowingSystem == "xcb" then
            defines { "VK_USE_PLATFORM_XCB_KHR" }

        elseif options.windowingSystem == "wayland" then
            defines { "VK_USE_PLATFORM_WAYLAND_KHR" }

        elseif options.windowingSystem == "dwm" then
            defines { "VK_USE_PLATFORM_WIN32_KHR" }

        else
            error("Unsupported windowing system " .. options.windowingSystem)

        end

        useChamber()
        useGlm()
        useOpenvr() -- Needed because it is a shared lib, while vulkan and shaderc are build statically
        useVulkan() -- Just adding pthread option
    end

    buildWithOpenvr()
    buildWithShaderc()
    buildWithVulkan()
    magmaDependencies()

    function useMagma()
        includedirs "magma"
        links { "lava-magma" }

        magmaDependencies()
    end
