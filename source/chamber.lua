project "lava-chamber"
    kind "SharedLib"
    pic "on"

    files "chamber/**"

    function chamberDependencies()
    end

    chamberDependencies()

    function useChamber()
        links "lava-chamber"

        chamberDependencies()
    end
