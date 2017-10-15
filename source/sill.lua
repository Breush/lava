project "lava-sill"
    kind "SharedLib"
    pic "on"

    files "sill/**"

    function sillDependencies()
        useMagma()
    end

    sillDependencies()

    function useSill()
        includedirs "sill"
        links { "lava-sill" }

        sillDependencies()
    end
