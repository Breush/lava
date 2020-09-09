local NAME = "Shaderc"
local VERSION = "2020.3"

-- Set up

if not fileExists("./.tmp/shaderc/" .. VERSION .. ".txt") then
    checkProgram("python", NAME)
    checkProgram("cmake", NAME)
    checkProgram("make", NAME)

    print("[Dependencies] Setting " .. NAME .. " (" .. VERSION .. ") up...")

    if not os.execute("bash ../scripts/setup/shaderc.sh") then
        error("[Dependencies] Cannot set " .. NAME .. " up.")
    end
end

-- Use hook

local externalPath = path.getabsolute(".")
function buildWithShaderc()
    includedirs(externalPath .. "/include")
    libdirs(externalPath .. "/lib")

    links { "shaderc_combined" }
end

print("[Dependencies] " .. NAME .. " (" .. VERSION .. ") is ready.")
