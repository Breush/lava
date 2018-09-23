project "lava-sill"
    kind "SharedLib"
    pic "on"
    
    pchheader "sill/pch.hpp"
    files "sill/**"

    function sillDependencies()
        useCrater()
        useMagma()
        useDike()
        useNlohmannJson()
    end

    sillDependencies()

    function useSill()
        includedirs "sill"
        links { "lava-sill" }

        sillDependencies()
    end
