project "lava-crater"
    kind "SharedLib"
    pic "on"

    files "crater/**"

    function craterDependencies()
        if os.get() == "linux" then
            defines { "VK_USE_PLATFORM_XCB_KHR" }
            local libXCB = os.findlib "xcb"
            if not libXCB then
                error "XCB dev files are required, please install libxcb1-dev libxcb-keysyms1-dev"
            end
            libdirs(libXCB)
            links { "xcb", "xcb-keysyms" }

        elseif os.get() == "windows" then
            defines { "VK_USE_PLATFORM_WINDOWS_KHR" }

        else
            error("Unsupported platform " + os.get())

        end

        useChamber()
    end

    craterDependencies()

    function useCrater()
        links "lava-crater"

        craterDependencies()
    end
