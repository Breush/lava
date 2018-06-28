project "lava-flow"
    kind "SharedLib"
    pic "on"

    files "flow/**"
    excludes "flow/backends/**"

    if options.audioSystem == "pulse" then
        defines { "LAVA_FLOW_AUDIO_PULSE" }
        files "flow/backends/pulse/**"

    else
        error("Unsupported audio system " .. options.audioSystem)

    end

    function flowDependencies()
        if options.audioSystem == "pulse" then
            local lib = os.findlib "pulse"
            if not lib then
                error("PulseAudio dev files are required, please install libpulse-dev.")
            end
            libdirs(lib)
            links { "pulse", "pulse-simple" }

        else
            error("Unsupported audio system " .. options.audioSystem)

        end

        useChamber()
    end

    flowDependencies()

    function useFlow()
        links "lava-flow"

        flowDependencies()
    end
