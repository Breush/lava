project "lava-dike"
    kind "SharedLib"
    pic "on"

    files "dike/**"

    function dikeDependencies()
        useChamber()
        useBullet()
    end

    dikeDependencies()

    function useDike()
        includedirs "dike"
        links { "lava-dike" }

        dikeDependencies()
    end
