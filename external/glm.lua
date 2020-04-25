-- UPDATE THESE WHENEVER NEEDED

local NAME = "GLM"
local VERSION = "0.9.9.8"

-- Download

local localFile = "./.tmp/glm_" .. VERSION .. ".zip"
if not fileValid(localFile) then
    os.mkdir("./.tmp")
    os.execute('bash -c "rm --recursive ./include/glm 2> /dev/null"')
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

    if not os.execute("bash ../scripts/setup/glm.sh " .. VERSION) then
        error("[Dependencies] Cannot set " .. NAME .. " up.")
    end
end

-- Use hook

local externalPath = path.getabsolute(".")
function useGlm()
    includedirs(externalPath .. "/include")

    defines { "GLM_FORCE_DEPTH_ZERO_TO_ONE" }
end

print("[Dependencies] " .. NAME .. " (" .. VERSION .. ") is ready.")
