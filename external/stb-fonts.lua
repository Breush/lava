-- Download

if not fileValid("./include/stb/consolas_latin1.inl") then
    print("[Dependencies] Downloading STB Fonts...")

    os.mkdir("./include/stb")

    local url = "http://nothings.org/stb/font/latin1/consolas/stb_font_consolas_24_latin1.inl"
    local downloadResult = http.download(url, "./include/stb/consolas_latin1.inl")

    if downloadResult ~= "OK" then
        print("[Dependencies] FAILURE while downloading STB Fonts (" .. VULKAN_SDK_VERSION .. ")...")
        print("If it persists, please try downloading " .. url .. " by yourself")
        print(downloadResult)
        os.exit(1)
    end
end

-- Use hook

local externalPath = path.getabsolute(".")
function useStbFonts()
    includedirs(externalPath .. "/include")
end

print("[Dependencies] STB Fonts are ready.")

