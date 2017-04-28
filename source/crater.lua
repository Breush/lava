project "lava-crater"
    kind "SharedLib"
    pic "on"

    files "crater/**"

    function craterDependencies()
        -- TODO To be changed according to platform
        -- And define should probably not be in here
        defines { "VK_USE_PLATFORM_XCB_KHR" }
        local libXCB = os.findlib "xcb"
        if not libXCB then
            error("XCB dev files are required, please install libxcb1-dev")
        end
        libdirs(libXCB)
        links { "xcb", "GL" }

        useChamber()
    end

    craterDependencies()

    function useCrater()
        links "lava-crater"

        craterDependencies()
    end
