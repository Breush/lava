project "lava-chamber"
    kind "SharedLib"
    pic "on"

    files "chamber/**"
    excludes "chamber/call-stack/**"

    if os.host() == "linux" then
        defines { "LAVA_CHAMBER_CALLSTACK_GCC" }
        files "chamber/call-stack/gcc/**"

    elseif os.host() == "windows" then
        defines { "LAVA_CHAMBER_CALLSTACK_MINGW" }
        files "chamber/call-stack/mingw/**"

    else
        error("Unsupported platform " + os.host())

    end

    function chamberDependencies()
        if os.host() == "windows" then
            buildoptions { "-mwindows", "-municode" }
            links { "DbgHelp" }
        
        end

        links { "pthread" }

        useGlm()
        useStb()
    end

    chamberDependencies()

    function useChamber()
        links { "lava-chamber" }

        chamberDependencies()
    end
