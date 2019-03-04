-- UPDATE THESE WHENEVER NEEDED

local NAME = "Vulkan SDK"
local VERSION = "1.1.101.0"

-- Download

local localFile = "./.tmp/vulkan-sdk_" .. VERSION .. ".tar.gz";
if not fileValid(localFile) then
    checkProgramsAny({"make", "mingw32-make"}, NAME)
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

    os.execute("bash ../scripts/setup/vulkan-sdk.sh " .. VERSION)
end

-- Use hook

local externalPath = path.getabsolute(".")
function useVulkanSdk()
    includedirs(externalPath .. "/include")
    libdirs(externalPath .. "/lib")

    if os.host() == "windows" then
        links { "vulkan-1" }
        links {
            "shadercd", "shaderc_utild",
            "glslangd", "OSDependentd", "OGLCompilerd",
            "SPIRVd", "HLSLd", "SPIRV-Tools-optd", "SPIRV-Toolsd",
        }
    else
        links { "vulkan" }
        links {
            "shaderc", "shaderc_util",
            "glslang", "OSDependent", "OGLCompiler",
            "SPIRV", "HLSL", "SPIRV-Tools-opt", "SPIRV-Tools",
        }
    end


    linkoptions("-pthread")

    filter { "configurations:debug" }
        linkoptions("-Wl,-rpath," .. externalPath .. "/lib")

    filter {}
end

print("[Dependencies] " .. NAME .. " (" .. VERSION .. ") is ready.")
