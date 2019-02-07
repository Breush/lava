project "lava-flow"
    kind "SharedLib"
    pic "on"

    pchheader "flow/pch.hpp"
    files "flow/**"
    excludes "flow/backends/**"

    if options.audioSystem == "pulse" then
        defines { "LAVA_FLOW_AUDIO_PULSE" }
        files "flow/backends/pulse/**"

    elseif options.audioSystem == "was" then
        defines { "LAVA_FLOW_AUDIO_WAS" }

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

        elseif options.audioSystem == "was" then
            -- Nothing to do

        else
            error("Unsupported audio system " .. options.audioSystem)

        end

        useGlm()
        useChamber()
    end

    flowDependencies()

    function useFlow()
        links "lava-flow"

        flowDependencies()
    end
