project "lava-magma"
    kind "SharedLib"
    pic "on"

    files "magma/**"

    function magmaDependencies()
        if os.host() == "linux" then
            defines { "VK_USE_PLATFORM_XCB_KHR" }

        elseif os.host() == "windows" then
            defines { "VK_USE_PLATFORM_WIN32_KHR" }

        else
            error("Unsupported platform " + os.host())

        end

        useCrater()
        useVulkanSdk()
        useNlohmannJson()
    end

    magmaDependencies()

    function useMagma()
        includedirs "magma"
        links { "lava-magma" }

        magmaDependencies()
    end
