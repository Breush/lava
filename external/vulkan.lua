local NAME = "Vulkan"
local VERSION = "1.2.162"

-- Set up

if not fileExists("./.tmp/vulkan/" .. VERSION .. ".txt") then
    checkProgram("cmake", NAME)
    checkProgram("make", NAME)

    print("[Dependencies] Setting " .. NAME .. " (" .. VERSION .. ") up...")

    if not os.execute("bash ../scripts/setup/vulkan.sh") then
        error("[Dependencies] Cannot set " .. NAME .. " up.")
    end
end

-- Use hook

local externalPath = path.getabsolute(".")
function useVulkan()
    linkoptions("-pthread")
end

function buildWithVulkan()
    includedirs(externalPath .. "/include")
    libdirs(externalPath .. "/lib")

    if os.host() == "windows" then
        links { "vulkan-1" }
    else
        links { "vulkan" }
    end

    defines { "VULKAN_HPP_DISPATCH_LOADER_DYNAMIC",
              "VULKAN_HPP_NO_NODISCARD_WARNINGS",
              "VULKAN_HPP_NO_EXCEPTIONS" }

    useVulkan()
end

print("[Dependencies] " .. NAME .. " (" .. VERSION .. ") is ready.")
