----------------------------------
-- Ashes - Example applications --
----------------------------------

-------------
-- chamber --

project "chamber-watcher"
    kind "WindowedApp"
    files "chamber/watcher.cpp"
    useChamber()

------------
-- crater --

project "crater-events"
    kind "WindowedApp"
    files "crater/events.cpp"
    useCrater()

-----------
-- magma --

project "magma-instancing"
    kind "WindowedApp"
    files "magma/instancing.cpp"
    useCrater()
    useMagma()

project "magma-ray-tracer"
    kind "WindowedApp"
    files "magma/ray-tracer.cpp"
    useCrater()
    useMagma()

project "magma-scenes-and-windows"
    kind "WindowedApp"
    files "magma/scenes-and-windows.cpp"
    useCrater()
    useMagma()

project "magma-translucency"
    kind "WindowedApp"
    files "magma/translucency.cpp"
    useCrater()
    useMagma()

project "magma-shader-watcher"
    kind "WindowedApp"
    files "magma/shader-watcher.cpp"
    useCrater()
    useMagma()

project "magma-shadows"
    kind "WindowedApp"
    files "magma/shadows.cpp"
    useCrater()
    useMagma()

project "magma-vr"
    kind "WindowedApp"
    files "magma/vr.cpp"
    useCrater()
    useMagma()

----------
-- dike --

project "dike-bouncy-sphere"
    kind "WindowedApp"
    files "dike/bouncy-sphere.cpp"
    useDike()

----------
-- flow --

project "flow-play-musics"
    kind "WindowedApp"
    files "flow/play-musics.cpp"
    useFlow()

project "flow-play-sounds"
    kind "WindowedApp"
    files "flow/play-sounds.cpp"
    useFlow()

project "flow-spatialization-effect"
    kind "WindowedApp"
    files "flow/spatialization-effect.cpp"
    useFlow()

----------
-- sill --

project "sill-matcap-material"
    kind "WindowedApp"
    files "sill/matcap-material.cpp"
    useSill()

project "sill-mesh-makers"
    kind "WindowedApp"
    files "sill/mesh-makers.cpp"
    useSill()

project "sill-physics-demo"
    kind "WindowedApp"
    files "sill/physics-demo.cpp"
    useSill()

project "sill-rm-material"
    kind "WindowedApp"
    files "sill/rm-material.cpp"
    useSill()

project "sill-sponza"
    kind "WindowedApp"
    files "sill/sponza.cpp"
    useSill()

project "sill-text"
    kind "WindowedApp"
    files "sill/text.cpp"
    useSill()

project "sill-ui"
    kind "WindowedApp"
    files "sill/ui.cpp"
    useSill()

project "sill-vr-sandbox"
    kind "WindowedApp"
    files "sill/vr-sandbox.cpp"
    useSill()

-----------
-- games --

project "sill-vr-puzzle"
    kind "WindowedApp"
    files "sill/vr-puzzle/**"
    useSill()

