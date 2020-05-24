local NAME = "GLM"
local VERSION = "0.9.9.8"

-- Set up

if not fileExists("./.tmp/glm/" .. VERSION .. ".txt") then
    print("[Dependencies] Setting " .. NAME .. " (" .. VERSION .. ") up...")

    if not os.execute("bash ../scripts/setup/glm.sh") then
        error("[Dependencies] Cannot set " .. NAME .. " up.")
    end
end

-- Use hook

local externalPath = path.getabsolute(".")
function useGlm()
    includedirs(externalPath .. "/include")

    defines { "GLM_FORCE_DEPTH_ZERO_TO_ONE" }
end

print("[Dependencies] " .. NAME .. " (" .. VERSION .. ") is ready.")
