project "lava-renderer"
    kind "StaticLib"

    files "lava-renderer/**"

    function lavaRendererDependencies()
        -- TODO To be changed according to platform
        -- And define should probably not be in here
        defines { "VK_USE_PLATFORM_XCB_KHR" }
        links "xcb"

        useVulkanSdk()
        useStbFonts()
        useGlm()
    end

    function useLavaRenderer()
        includedirs "lava-renderer"
        links "lava-renderer"

        lavaRendererDependencies()
    end

-- The app

project "app"
    kind "ConsoleApp"

    files "app/**"

    useLavaRenderer()
