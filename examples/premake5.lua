----------------------------------
-- Ashes - Example applications --
----------------------------------

----------
-- chamber --

project "chamber-watcher"
kind "WindowedApp"
files "chamber/watcher.cpp"
useChamber()

-----------
-- magma --

project "magma-scenes-and-windows"
    kind "WindowedApp"
    files "magma/scenes-and-windows.cpp"
    useMagma()
    
project "magma-transparency"
    kind "WindowedApp"
    files "magma/transparency.cpp"
    useMagma()
    
project "magma-shader-watcher"
    kind "WindowedApp"
    files "magma/shader-watcher.cpp"
    useMagma()

----------
-- dike --

project "dike-bouncy-sphere"
    kind "WindowedApp"
    files "dike/bouncy-sphere.cpp"
    useDike()

----------
-- sill --

project "sill-mesh-makers"
    kind "WindowedApp"
    files "sill/mesh-makers.cpp"
    useSill()

project "sill-rm-material"
    kind "WindowedApp"
    files "sill/rm-material.cpp"
    useSill()
    
project "sill-text"
    kind "WindowedApp"
    files "sill/text.cpp"
    useSill()
