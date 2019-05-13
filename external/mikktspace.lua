-- Download

local NAME = "MikkTSpace"
local REPOSITORY = "git@github.com:tcoppex/ext-mikktspace.git"

if not fileExists("./.tmp/mikktspace") then
    os.mkdir("./.tmp")
    os.execute('bash -c "rm --recursive ./include/mikktspace 2> /dev/null"')

    downloadStartBasic("Dependencies", NAME)
    local success = os.execute("git clone --depth=1 " .. REPOSITORY .. " .tmp/mikktspace 2> /dev/null")
    downloadStopBasic()

    if not success then
        print("[Dependencies] FAILURE while downloading " .. NAME .. "...")
        print("If it persists, please try cloning " .. REPOSITORY .. " by yourself")
        print("and move it to " .. path.getabsolute("./.tmp/mikktspace"))
        os.exit(1)
    end
end

-- Set up

if not fileExists("./include/mikktspace") then
    print("[Dependencies] Setting " .. NAME .. " up...")

    os.execute("bash ../scripts/setup/mikktspace.sh")
end

-- Use hook

local externalPath = path.getabsolute(".")
function useMikkTSpace()
    includedirs(externalPath .. "/include")
end

print("[Dependencies] " .. NAME .. " is ready.")
