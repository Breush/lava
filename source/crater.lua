project "lava-crater"
    kind "SharedLib"
    pic "on"

    files "crater/**"
    excludes "crater/window/**"

    if os.get() == "linux" then
        files "crater/window/xcb/**"

    elseif os.get() == "windows" then
        files "crater/window/dwm/**"

    else
        error("Unsupported platform " + os.get())

    end

    function craterDependencies()
        if os.get() == "linux" then
            defines { "LAVA_CRATER_WINDOW_XCB" }
            
            local libXCB = os.findlib "xcb"
            if not libXCB then
                error "XCB dev files are required, please install libxcb1-dev libxcb-keysyms1-dev"
            end
            libdirs(libXCB)
            links { "xcb", "xcb-keysyms" }

        elseif os.get() == "windows" then
            defines { "LAVA_CRATER_WINDOW_DWM" }
            buildoptions { "-mwindows", "-municode" }

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
