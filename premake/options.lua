local SETUP_FILE = "../.setup.json"

-- Read the last setup options
options = json.decode(io.readfile(SETUP_FILE))

function setOption(name, value)
    if value == nil or value == options[name] then
        return
    end

    options[name] = value

    -- Rewrite setup options
    io.writefile(SETUP_FILE, json.encode_pretty(options))
end

-- Some default values
if os.host() == "windows" then
    setOption("windowingSystem", "dwm")
end

-- Add command-line options
setOption("windowingSystem", _OPTIONS["windowing-system"])
