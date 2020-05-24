local NAME = "OpenVR"
local VERSION = "1.11.11"

-- Set up

if not fileExists("./.tmp/openvr/" .. VERSION .. ".txt") then
    print("[Dependencies] Setting " .. NAME .. " (" .. VERSION .. ") up...")

    if not os.execute("bash ../scripts/setup/openvr.sh") then
        error("[Dependencies] Cannot set " .. NAME .. " up.")
    end
end

-- Use hook

local externalPath = path.getabsolute(".")
function useOpenvr()
    libdirs(externalPath .. "/lib")

    links { "openvr_api" }
end

function buildWithOpenvr()
    includedirs(externalPath .. "/include")

    if os.host() == "windows" then
        postbuildcommands  { "cp " .. externalPath .. "/lib/openvr_api* %{cfg.targetdir}/" }
    end

    useOpenvr()
end

print("[Dependencies] " .. NAME .. " (" .. VERSION .. ") is ready.")
