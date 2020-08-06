local NAME = "STB libraries"
local REPOSITORY = "https://github.com/nothings/stb.git"

-- Set up

if not fileExists("./.tmp/stb/repository.txt") then
    print("[Dependencies] Setting " .. NAME .. " up...")

    if not os.execute("bash ../scripts/setup/stb.sh") then
        error("[Dependencies] Cannot set " .. NAME .. " up.")
    end
end

-- Use hook

local externalPath = path.getabsolute(".")
function useStb()
    includedirs(externalPath .. "/include")
end

print("[Dependencies] " .. NAME .. " are ready.")

