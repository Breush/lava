project "lava-chamber"
    kind "SharedLib"
    
    -- Ignored by clang
    -- pic "on"

    files "chamber/**"

    function chamberDependencies()
        links { "pthread" }

        useGlm()
        useStb()
    end

    chamberDependencies()

    function useChamber()
        links { "lava-chamber" }

        chamberDependencies()
    end
