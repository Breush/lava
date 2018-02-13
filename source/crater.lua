project "lava-crater"
    kind "SharedLib"
    pic "on"

    files "crater/**"
    excludes "crater/window/**"

    -- No dependencies in the public interface
    useGlm()

    if os.host() == "linux" then
        defines { "LAVA_CRATER_WINDOW_XCB" }
        files "crater/window/xcb/**"

    elseif os.host() == "windows" then
        defines { "LAVA_CRATER_WINDOW_DWM" }
        files "crater/window/dwm/**"

    else
        error("Unsupported platform " + os.host())

    end

    function craterDependencies()
        if os.host() == "linux" then
            local libXCB = os.findlib "xcb"
            if not libXCB then
                error "XCB dev files are required, please install libxcb1-dev libxcb-keysyms1-dev"
            end
            libdirs(libXCB)
            links { "xcb", "xcb-keysyms" }

        elseif os.host() == "windows" then
            buildoptions { "-mwindows", "-municode" }

        else
            error("Unsupported platform " + os.host())

        end
    end

    craterDependencies()

    function useCrater()
        links "lava-crater"

        craterDependencies()
    end
