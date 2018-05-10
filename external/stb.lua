-- Download

local NAME = "STB libraries"
local REPOSITORY = "git@github.com:nothings/stb.git"

if not fileExists("./.tmp/stb") then
    os.mkdir("./.tmp")
    os.execute('bash -c "rm --recursive ./include/stb 2> /dev/null"')
    
    downloadStartBasic("Dependencies", NAME)
    local success = os.execute("git clone --depth=1 " .. REPOSITORY .. " .tmp/stb 2> /dev/null")

    if not success then
        downloadStopBasic()
        print("[Dependencies] FAILURE while downloading " .. NAME .. "...")
        print("If it persists, please try cloning " .. REPOSITORY .. " by yourself")
        print("and move it to " .. path.getabsolute("./.tmp/stb"))
        os.exit(1)
    end
end

-- Set up

if not fileExists("./include/stb") then
    print("[Dependencies] Setting " .. NAME .. " up...")

    os.execute("bash ../scripts/setup/stb.sh")
end

-- Use hook

local externalPath = path.getabsolute(".")
function useStb()
    includedirs(externalPath .. "/include")
end

print("[Dependencies] " .. NAME .. " are ready.")

