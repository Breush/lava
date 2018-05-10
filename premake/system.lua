function doesProgramExists(program)
    return os.execute('bash -c "' .. program .. ' --version > /dev/null 2>&1"')
end

-- Check that a program exists on the system
function checkProgram(program, dependency)
    if not doesProgramExists(program) then
        error(program .. " is required to install " .. dependency .. " but was not found.")
    end

    return program
end

-- Check that at least one of the programs exist on the system
function checkProgramsAny(programs, dependency)
    for i, program in ipairs(programs) do
        if doesProgramExists(program) then
            return program
        end
    end

    error("Any of the programs " .. json.encode(programs) .. " is required to install " .. dependency .. " but none was found.")
end
