project "lava-magma"
    kind "SharedLib"
    pic "on"

    files "magma/**"

    function magmaDependencies()
        -- TODO To be changed according to platform
        -- And define should probably not be in here
        defines { "VK_USE_PLATFORM_XCB_KHR" }
        local libXCB = os.findlib "xcb"
        if not libXCB then
            error("XCB dev files are required, please install libxcb1-dev")
        end
        libdirs(libXCB)
        links "xcb"

        useVulkanSdk()
        useStbFonts()
        useGlm()
    end

    magmaDependencies()

    function useMagma()
        includedirs "magma"
        links "lava-magma"

        magmaDependencies()
    end
