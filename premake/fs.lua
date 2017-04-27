-- Tool function to get a file's size
function fileSize(fileName)
    local file = io.open(fileName, "rb")
    local size = file:seek("end")
    file:close()
    return size
end

--- Tool function to check if a file or folder exists
function fileExists(fileName)
    local f = os.rename(fileName, fileName)
    return f ~= nil
end

--- Tool function to check if a file exists and is not empty
function fileValid(fileName)
    return fileExists(fileName) and fileSize(fileName) ~= 0
end
