project "lava-crater"
    kind "SharedLib"
    pic "on"

    pchheader "crater/pch.hpp"
    files "crater/**"
    excludes "crater/window/**"

    if options.windowingSystem == "xcb" then
        defines { "LAVA_CRATER_WINDOW_XCB" }
        files "crater/window/xcb/**"

    elseif options.windowingSystem == "wayland" then
        defines { "LAVA_CRATER_WINDOW_WAYLAND" }
        files "crater/window/wayland/**"

    elseif options.windowingSystem == "dwm" then
        defines { "LAVA_CRATER_WINDOW_DWM" }
        files "crater/window/dwm/**"

    else
        error("Unsupported windowing system " .. options.windowingSystem)

    end

    function craterDependencies()
        if options.windowingSystem == "xcb" then
            local lib = (os.findlib "xcb") and (os.findlib "xcb-xkb") and (os.findlib "xkbcommon")
            if not lib then
                error("XCB dev files are required, please install libxcb1-dev libxcb-xkb1 libxkbcommon-dev libxkbcommon-x11-dev.\n" ..
                      "If you prefer to use Wayland, run `./scripts/setup.sh --windowing-system=wayland`.")
            end
            libdirs(lib)
            links { "xcb", "xcb-xkb", "xkbcommon", "xkbcommon-x11" }

        elseif options.windowingSystem == "wayland" then
            local lib = os.findlib "wayland-client"
            if not lib then
                error("Wayland dev files are required, please install libwayland-dev.\n" ..
                      "If you prefer to use XCB, run `./scripts/setup.sh --windowing-system=xcb`.")
            end
            libdirs(lib)
            links { "wayland-client" }

        elseif options.windowingSystem == "dwm" then
            buildoptions { "-mwindows", "-municode" }

        else
            error("Unsupported windowing system " .. options.windowingSystem)

        end

        useGlm()
        useChamber()
    end

    craterDependencies()

    function useCrater()
        links "lava-crater"

        craterDependencies()
    end
