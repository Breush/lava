-- UPDATE THESE WHENEVER NEEDED

local NAME = "GLM"
local VERSION = "0.9.9-a2"

-- Download

local localFile = "./.tmp/glm_" .. VERSION .. ".zip"
if not fileValid(localFile) then
    os.mkdir("./.tmp")
    os.execute("rm --recursive ./include/glm")
    local url = "https://github.com/g-truc/glm/archive/" .. VERSION .. ".zip"
    
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

if not fileExists("./include/glm") then
    print("[Dependencies] Setting " .. NAME .. " (" .. VERSION .. ") up...")
    os.execute("cd ./.tmp && unzip -o glm_" .. VERSION .. ".zip && cp -r glm/glm ../include")
end

-- Use hook

local externalPath = path.getabsolute(".")
function useGlm()
    includedirs(externalPath .. "/include")

    defines { "GLM_FORCE_RADIANS", "GLM_FORCE_DEPTH_ZERO_TO_ONE" }
end

print("[Dependencies] " .. NAME .. " (" .. VERSION .. ") is ready.")
