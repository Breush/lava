local NAME = "MikkTSpace"
local REPOSITORY = "https://github.com/tcoppex/ext-mikktspace.git"

-- Set up

if not fileExists("./.tmp/mikktspace/repository.txt") then
    print("[Dependencies] Setting " .. NAME .. " up...")

    if not os.execute("bash ../scripts/setup/mikktspace.sh") then
        error("[Dependencies] Cannot set " .. NAME .. " up.")
    end
end

-- Use hook

local externalPath = path.getabsolute(".")
function useMikkTSpace()
    includedirs(externalPath .. "/include")
end

print("[Dependencies] " .. NAME .. " is ready.")
