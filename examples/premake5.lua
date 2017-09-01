----------------------------------
-- Ashes - Example applications --
----------------------------------

project "magma-load-meshes"
    kind "WindowedApp"
    files "magma/load-meshes.cpp"
    useMagma()

project "magma-rm-material"
    kind "WindowedApp"
    files "magma/rm-material.cpp"
    useMagma()

project "magma-scenes-and-windows"
    kind "WindowedApp"
    files "magma/scenes-and-windows.cpp"
    useMagma()
