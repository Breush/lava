project "lava-magma"
    kind "SharedLib"
    pic "on"

    files "magma/**"

    function magmaDependencies()
        -- TODO To be changed according to platform
        -- And define should probably not be in here
        defines { "VK_USE_PLATFORM_XCB_KHR" }

        useCrater()
        useVulkanSdk()
        useStbFonts()
    end

    magmaDependencies()

    function useMagma()
        includedirs "magma"
        links { "lava-magma" }

        magmaDependencies()
    end
