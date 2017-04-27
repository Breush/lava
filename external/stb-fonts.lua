-- Download

if not fileValid("./include/stb/consolas_latin1.inl") then
    os.mkdir("./include/stb")

    local url = "http://nothings.org/stb/font/latin1/consolas/stb_font_consolas_24_latin1.inl"
    
    downloadStart("Dependencies", "STB Fonts")
    local downloadResult = http.download(url, "./include/stb/consolas_latin1.inl", { progress = downloadProgress })

    if downloadResult ~= "OK" then
        print("[Dependencies] FAILURE while downloading STB Fonts...")
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

