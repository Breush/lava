project "lava-chamber"
    kind "SharedLib"
    pic "on"

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
