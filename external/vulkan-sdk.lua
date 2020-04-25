-- UPDATE THESE WHENEVER NEEDED

local NAME = "Vulkan SDK"
local VERSION = "1.2.135.0"

-- Download

local localFile = "./.tmp/vulkan-sdk_" .. VERSION .. ".tar.gz";
if not fileValid(localFile) then
    checkProgram("make", NAME)
    checkProgram("cmake", NAME)
    checkProgram("python", NAME)

    os.mkdir("./.tmp/")
    os.execute('bash -c "rm --recursive ./include/vulkan 2> /dev/null"')

    local filename = "linux/vulkansdk-linux-x86_64-" .. VERSION .. ".tar.gz"
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

    if not os.execute("bash ../scripts/setup/vulkan-sdk.sh") then
        error("[Dependencies] Cannot set " .. NAME .. " up.")
    end
end

-- Use hook

local externalPath = path.getabsolute(".")
function useVulkanSdk()
    includedirs(externalPath .. "/include")
    libdirs(externalPath .. "/lib")

    if os.host() == "windows" then
        links { "vulkan-1" }
        links {
            "shaderc", "shaderc_util",
            "glslangd", "OSDependentd", "OGLCompilerd",
            "SPIRVd", "HLSLd", "SPIRV-Tools-opt", "SPIRV-Tools",
        }
    else
        links { "vulkan" }
        links { "shaderc_shared" }
    end

    linkoptions("-pthread")
    defines { "VULKAN_HPP_NO_EXCEPTIONS" }

    filter { not "configurations:release" }
        linkoptions("-Wl,-rpath," .. externalPath .. "/lib")

    filter {}
end

print("[Dependencies] " .. NAME .. " (" .. VERSION .. ") is ready.")
