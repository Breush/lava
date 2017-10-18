----------------------------------
-- Ashes - Example applications --
----------------------------------

project "magma-scenes-and-windows"
    kind "WindowedApp"
    files "magma/scenes-and-windows.cpp"
    useMagma()

project "magma-transparency"
    kind "WindowedApp"
    files "magma/transparency.cpp"
    useMagma()

project "sill-mesh-makers"
    kind "WindowedApp"
    files "sill/mesh-makers.cpp"
    useSill()

project "sill-rm-material"
    kind "WindowedApp"
    files "sill/rm-material.cpp"
    useSill()
