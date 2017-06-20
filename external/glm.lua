-- UPDATE THESE WHENEVER NEEDED

local GLM_VERSION = "0.9.8.4"

-- Download

local localFile = "./.tmp/glm_" .. GLM_VERSION .. ".zip"
if not fileValid(localFile) then
    os.mkdir("./.tmp")
    os.remove("./include/glm")
    local url = "https://github.com/g-truc/glm/releases/download/" .. GLM_VERSION .. "/glm-" .. GLM_VERSION .. ".zip"
    
    downloadStart("Dependencies", "GLM (" .. GLM_VERSION .. ")")
    local downloadResult = http.download(url, localFile, { progress = downloadProgress })

    if downloadResult ~= "OK" then
        downloadStop()
        print("[Dependencies] FAILURE while downloading GLM (" .. GLM_VERSION .. ")...")
        print("If it persists, please try downloading " .. url .. " by yourself")
        print("and move it to " .. path.getabsolute(localFile))
        print(downloadResult)
        os.exit(1)
    end
end

-- Set up

if not fileExists("./include/glm") then
    print("[Dependencies] Setting GLM (" .. GLM_VERSION .. ") up...")
    os.execute("cd ./.tmp && unzip -o ./glm.zip && cp -r glm/glm ../include")
end

-- Use hook

local externalPath = path.getabsolute(".")
function useGlm()
    includedirs(externalPath .. "/include")

    defines { "GLM_FORCE_RADIANS", "GLM_FORCE_DEPTH_ZERO_TO_ONE" }
end

print("[Dependencies] GLM (" .. GLM_VERSION .. ") is ready.")
