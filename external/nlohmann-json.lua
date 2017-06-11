-- UPDATE THESE WHENEVER NEEDED

local NAME = "Nlohmann JSON"
local VERSION = "2.1.1"

-- Download

if not fileValid("./include/nlohmann/json.hpp") then
    os.mkdir("./include/nlohmann")
    local url = "https://github.com/nlohmann/json/releases/download/v" .. VERSION.. "/json.hpp"
    
    downloadStart("Dependencies", NAME .. " (" .. VERSION .. ")")
    local downloadResult = http.download(url, "./include/nlohmann/json.hpp", { progress = downloadProgress })

    if downloadResult ~= "OK" then
        downloadStop()
        print("[Dependencies] FAILURE while downloading GLM (" .. VERSION .. ")...")
        print("If it persists, please try downloading " .. url .. " by yourself")
        print("and move it to " .. path.getabsolute("./include/nlohmann"))
        print(downloadResult)
        os.exit(1)
    end
end

-- Use hook

local externalPath = path.getabsolute(".")
function useNlohmannJson()
    includedirs(externalPath .. "/include")
end

print("[Dependencies] " .. NAME .. " (" .. VERSION .. ") is ready.")
