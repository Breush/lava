-- UPDATE THESE WHENEVER NEEDED

local NAME = "Vulkan SDK"
local VERSION = "1.1.70.0"

-- Download

local localFile = "./.tmp/vulkan-sdk_" .. VERSION .. ".run";
if not fileValid(localFile) then
    os.mkdir("./.tmp/")
    os.execute("rm --recursive ./include/vulkan")

    local filename = "linux/vulkansdk-linux-x86_64-" .. VERSION .. ".run"
    if os.host() == "windows" then
        filename = "windows/VulkanSDK-" .. VERSION .. ".run"
    end
    
    local url = "https://vulkan.lunarg.com/sdk/download/" .. VERSION .. "/" .. filename .. "?Human=true"

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

    if os.host() == "windows" then
        local installFolder = path.getabsolute("./.tmp/vulkan-sdk"):gsub("/", "\\")
        os.execute("mv " .. localFile .. " vulkan-sdk.exe")
        os.execute("vulkan-sdk.exe /S /D=" .. installFolder)
        os.execute("mv vulkan-sdk.exe " .. localFile)
    end
    
    os.execute("bash ../scripts/setup/vulkan-sdk.sh " .. VERSION)
end

-- Use hook

local externalPath = path.getabsolute(".")
function useVulkanSdk()
    includedirs(externalPath .. "/include")
    libdirs(externalPath .. "/lib")

    if os.host() == "windows" then 
        links { "vulkan-1" }
    else
        links { "vulkan" }
    end

    links {
        "shaderc", "shaderc_util",
        "glslang", "OSDependent", "OGLCompiler",
        "SPIRV", "SPIRV-Toolsd", "SPIRV-Tools-optd",
        "SPVRemapper", "HLSL",       
    }

    linkoptions("-pthread")
    
    filter { "configurations:debug" }
        linkoptions("-Wl,-rpath," .. externalPath .. "/lib")

    filter {}
end

print("[Dependencies] " .. NAME .. " (" .. VERSION .. ") is ready.")
