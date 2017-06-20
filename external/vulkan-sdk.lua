-- UPDATE THESE WHENEVER NEEDED

local NAME = "Vulkan SDK"
local VERSION = "1.0.51.0"

-- Download

local localFile = "./.tmp/vulkan-sdk_" .. VERSION .. ".run";
if not fileValid(localFile) then
    os.mkdir("./.tmp/")
    os.remove("./include/vulkan")
    local filename = "vulkansdk-linux-x86_64-" .. VERSION .. ".run"
    local url = "https://vulkan.lunarg.com/sdk/download/" .. VERSION .. "/linux/" .. filename .. "?Human=true"

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

if not fileExists("./include/vulkan") then
    print("[Dependencies] Setting " .. NAME .. " (" .. VERSION .. ") up...")
    local folder = "VulkanSDK/" .. VERSION .. "/x86_64"
    os.execute("cd ./.tmp && bash ./vulkan-sdk_" .. VERSION .. ".run &&" ..
               "cp -R " .. folder .. "/include/vulkan ../include &&" ..
               "cp -R " .. folder .. "/lib/* ../lib &&" ..
               "cp -R " .. folder .. "/bin/* ../bin &&" ..
               "cp -R " .. folder .. "/etc/* ../etc")
end

-- Use hook

local externalPath = path.getabsolute(".")
function useVulkanSdk()
    includedirs(externalPath .. "/include")
    libdirs(externalPath .. "/lib")
    links "vulkan"
    
    filter { "configurations:debug" }
        linkoptions("-Wl,-rpath," .. externalPath .. "/lib")

    filter {}
end

print("[Dependencies] " .. NAME .. " (" .. VERSION .. ") is ready.")
