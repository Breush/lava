-- UPDATE THESE WHENEVER NEEDED

local NAME = "Nlohmann JSON"
local VERSION = "3.5.0"

-- Download

local localFile = "./.tmp/nlhomann-json_" .. VERSION .. ".hpp"
if not fileValid(localFile) then
    os.mkdir("./.tmp")
    os.execute('bash -c "rm --recursive ./include/nlohmann 2> /dev/null"')
    local url = "https://github.com/nlohmann/json/releases/download/v" .. VERSION .. "/json.hpp"
    
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

if not fileExists("./include/nlohmann") then
    print("[Dependencies] Setting " .. NAME .. " (" .. VERSION .. ") up...")
    os.mkdir("./include/nlohmann")
    os.execute("cp " .. localFile .. " ./include/nlohmann/json.hpp")
end

-- Use hook

local externalPath = path.getabsolute(".")
function useNlohmannJson()
    includedirs(externalPath .. "/include")
end

print("[Dependencies] " .. NAME .. " (" .. VERSION .. ") is ready.")
