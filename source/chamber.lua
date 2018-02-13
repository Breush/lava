project "lava-chamber"
    kind "SharedLib"
    pic "on"

    files "chamber/**"
    excludes "chamber/call-stack/**"
    excludes "chamber/file-watcher/**"

    if os.host() == "linux" then
        defines { "LAVA_CHAMBER_CALLSTACK_GCC" }
        files "chamber/call-stack/gcc/**"

        defines { "LAVA_CHAMBER_FILEWATCHER_INOTIFY" }
        files "chamber/file-watcher/inotify/**"

    elseif os.host() == "windows" then
        defines { "LAVA_CHAMBER_CALLSTACK_MINGW" }
        files "chamber/call-stack/mingw/**"

        defines { "LAVA_CHAMBER_FILEWATCHER_WIN32" }
        files "chamber/file-watcher/win32/**"

    else
        error("Unsupported platform " + os.host())

    end

    function chamberDependencies()
        if os.host() == "linux" then
            links { "dl" }

        elseif os.host() == "windows" then
            buildoptions { "-mwindows", "-municode" }
            links { "DbgHelp" }
        
        end

        links { "pthread" }

        useStb()
    end

    -- This is a static lib, thus it shouldn't be linked multiple times
    links { "stdc++fs" }

    chamberDependencies()

    function useChamber()
        links { "lava-chamber" }

        chamberDependencies()
    end
