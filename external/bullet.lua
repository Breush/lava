-- UPDATE THESE WHENEVER NEEDED

local NAME = "Bullet"
local VERSION = "2.88"

-- Download

local localFile = "./.tmp/bullet_" .. VERSION .. ".zip";
if not fileValid(localFile) then
    checkProgram("make", NAME)
    checkProgram("cmake", NAME)

    os.mkdir("./.tmp/")
    os.execute('bash -c "rm --recursive ./include/bullet 2> /dev/null"')

    local url = "https://github.com/bulletphysics/bullet3/archive/" .. VERSION .. ".zip"

    downloadStart("Dependencies", NAME .. " (" .. VERSION .. ")")
    local downloadResult = http.download(url, localFile, { progress = downloadProgress })

    if downloadResult ~= "OK" then
        downloadStop()
        print("[Dependencies] FAILURE while downloading " .. NAME .. " (" .. VERSION .. ")...")
        print("If it persists, please try downloading " .. url .. " by yourself")
        print("and move it to " .. path.getabsolute(localFile))
        print(downloadResult)
        os.exit(1)
    end
end

-- Set up

if not fileExists("./include/bullet/btBulletCollisionCommon.h") then
    print("[Dependencies] Setting " .. NAME .. " (" .. VERSION .. ") up...")

    os.execute("bash ../scripts/setup/bullet.sh " .. VERSION)
end

-- Use hook

local externalPath = path.getabsolute(".")
function useBullet()
    includedirs(externalPath .. "/include")

    -- Sadly necessary... as this how the bullet is internally referenced
    includedirs(externalPath .. "/include/bullet")

    libdirs(externalPath .. "/lib")

    links {
        "BulletDynamics",
        "BulletCollision",
        "LinearMath",
    }
end

print("[Dependencies] " .. NAME .. " (" .. VERSION .. ") is ready.")
