-- Check that a program exists on the system
function checkProgram(program, dependency)
    if not os.execute(program .. " --version > /dev/null 2>&1") then
        error(program .. " is required to install " .. dependency)
    end
end
