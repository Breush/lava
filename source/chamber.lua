project "lava-chamber"
    kind "SharedLib"
    pic "on"

    files "chamber/**"
    excludes "chamber/call-stack/**"

    if os.get() == "linux" then
        defines { "LAVA_CHAMBER_CALLSTACK_GCC" }
        files "chamber/call-stack/gcc/**"

    elseif os.get() == "windows" then
        defines { "LAVA_CHAMBER_CALLSTACK_MINGW" }
        files "chamber/call-stack/mingw/**"

    else
        error("Unsupported platform " + os.get())

    end

    function chamberDependencies()
        if os.get() == "windows" then
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
