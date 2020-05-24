local NAME = "Nlohmann JSON"
local VERSION = "3.7.3"

-- Set up

if not fileExists("./.tmp/nlohmann-json/" .. VERSION .. ".txt") then
    print("[Dependencies] Setting " .. NAME .. " (" .. VERSION .. ") up...")

    if not os.execute("bash ../scripts/setup/nlohmann-json.sh") then
        error("[Dependencies] Cannot set " .. NAME .. " up.")
    end
end

-- Use hook

local externalPath = path.getabsolute(".")
function useNlohmannJson()
    includedirs(externalPath .. "/include")
end

print("[Dependencies] " .. NAME .. " (" .. VERSION .. ") is ready.")
