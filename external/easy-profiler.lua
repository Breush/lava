-- UPDATE THESE WHENEVER NEEDED

local NAME = "EasyProfiler"
local VERSION = "2.1.0"

-- Download

local localFile = "./.tmp/easy_profiler_" .. VERSION .. ".zip";
if not fileValid(localFile) then
    checkProgram("make", NAME)
    checkProgram("cmake", NAME)

    os.mkdir("./.tmp/")
    os.execute('bash -c "rm --recursive ./include/easy 2> /dev/null"')

    local url = "https://github.com/yse/easy_profiler/archive/v" .. VERSION .. ".zip"

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

if not fileExists("./include/easy/profiler.h") then
    print("[Dependencies] Setting " .. NAME .. " (" .. VERSION .. ") up...")

    os.execute("bash ../scripts/setup/easy-profiler.sh " .. VERSION)
end

-- Use hook

local externalPath = path.getabsolute(".")
function useEasyProfiler()
    includedirs(externalPath .. "/include")
    libdirs(externalPath .. "/lib")

    links {
        "easy_profiler",
    }
end

print("[Dependencies] " .. NAME .. " (" .. VERSION .. ") is ready.")
