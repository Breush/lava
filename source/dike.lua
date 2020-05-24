project "lava-dike"
    kind "SharedLib"
    pic "on"

    pchheader "dike/pch.hpp"
    files "dike/**"

    function dikeDependencies()
        useGlm()
        useChamber()
    end

    buildWithBullet()
    dikeDependencies()

    function useDike()
        includedirs "dike"
        links { "lava-dike" }

        dikeDependencies()
    end
