project "lava-dike"
    kind "SharedLib"
    pic "on"

    pchheader "dike/pch.hpp"
    files "dike/**"

    function dikeDependencies()
        useGlm()
        useChamber()
        useBullet()
    end

    dikeDependencies()

    function useDike()
        includedirs "dike"
        links { "lava-dike" }

        dikeDependencies()
    end
