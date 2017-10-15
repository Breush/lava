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
