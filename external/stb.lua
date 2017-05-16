-- Download

if not fileValid("./.tmp/stb") then
    os.mkdir("./.tmp")

    local url = "git@github.com:nothings/stb.git"
    
    downloadStart("Dependencies", "STB librairies")
    local errorCode = os.execute("git clone --depth=1 " .. url .. " .tmp/stb")

    if errorCode ~= 0 then
        print("[Dependencies] FAILURE while downloading STB librairies...")
        print("If it persists, please try downloading " .. url .. " by yourself")
        os.exit(1)
    end
end

-- Set up

if not fileExists("./include/stb/stb_image.h") then
    os.mkdir("./include/stb")

    print("[Dependencies] Setting STB librairies up...")
    os.copyfile(".tmp/stb/stb_image.h", "./include/stb")
end

-- Use hook

local externalPath = path.getabsolute(".")
function useStbFonts()
    includedirs(externalPath .. "/include")
    defines { "STB_IMAGE_IMPLEMENTATION" }
end

print("[Dependencies] STB librairies are ready.")

