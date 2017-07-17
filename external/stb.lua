-- Download

if not fileExists("./.tmp/stb") then
    os.mkdir("./.tmp")
    os.execute("rm --recursive ./include/stb")

    local url = "git@github.com:nothings/stb.git"
    
    downloadStart("Dependencies", "STB librairies")
    local errorCode = os.execute("git clone --depth=1 " .. url .. " .tmp/stb")

    if errorCode ~= 0 then
        downloadStop()
        print("[Dependencies] FAILURE while downloading STB librairies...")
        print("If it persists, please try cloning " .. url .. " by yourself")
        print("and move it to " .. path.getabsolute("./.tmp/stb"))
        os.exit(1)
    end
end

-- Set up

if not fileExists("./include/stb") then
    os.mkdir("./include/stb")

    print("[Dependencies] Setting STB librairies up...")
    os.copyfile(".tmp/stb/stb_image.h", "./include/stb")
end

-- Use hook

local externalPath = path.getabsolute(".")
function useStb()
    includedirs(externalPath .. "/include")
end

print("[Dependencies] STB librairies are ready.")

