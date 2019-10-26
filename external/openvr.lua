-- UPDATE THESE WHENEVER NEEDED

local NAME = "OpenVR"
local VERSION = "1.7.15"

-- Download

local localFile = "./.tmp/openvr_" .. VERSION .. ".zip";
if not fileValid(localFile) then
    checkProgram("make", NAME)
    checkProgram("cmake", NAME)

    os.mkdir("./.tmp/")
    os.execute('bash -c "rm --recursive ./include/openvr 2> /dev/null"')

    local url = "https://github.com/ValveSoftware/openvr/archive/v" .. VERSION .. ".zip"

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

if not fileExists("./include/openvr/openvr.h") then
    print("[Dependencies] Setting " .. NAME .. " (" .. VERSION .. ") up...")

    os.execute("bash ../scripts/setup/openvr.sh " .. VERSION)
end

-- Use hook

local externalPath = path.getabsolute(".")
function useOpenvr()
    includedirs(externalPath .. "/include")

    libdirs(externalPath .. "/lib")

    links {
        "openvr_api",
    }
end

print("[Dependencies] " .. NAME .. " (" .. VERSION .. ") is ready.")
