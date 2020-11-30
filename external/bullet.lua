local NAME = "Bullet"
local VERSION = "3.07"

-- Set up

if not fileExists("./.tmp/bullet/" .. VERSION .. ".txt") then
    checkProgram("cmake", NAME)
    checkProgram("make", NAME)

    print("[Dependencies] Setting " .. NAME .. " (" .. VERSION .. ") up...")

    if not os.execute("bash ../scripts/setup/bullet.sh") then
        error("[Dependencies] Cannot set " .. NAME .. " up.")
    end
end

-- Use hook

local externalPath = path.getabsolute(".")
function buildWithBullet()
    includedirs(externalPath .. "/include")

    -- Sadly necessary... as this how bullet is referenced internally
    includedirs(externalPath .. "/include/bullet")

    libdirs(externalPath .. "/lib")

    links {
        "BulletDynamics",
        "BulletCollision",
        "LinearMath",
    }
end

print("[Dependencies] " .. NAME .. " (" .. VERSION .. ") is ready.")
