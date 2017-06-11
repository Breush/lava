local downloadNextStep = 1
local downloadComplete = true

function downloadStart(kind, name)
	downloadNextStep = 0.01
	downloadComplete = false

	io.write("[" .. kind .. "] \27[s") -- Save cursor
	io.flush()

	io.write("---------- Downloading " .. name .. "...")
	io.write("\27[u") -- Restore cursor
	io.flush()
end

function downloadStop()
	io.write("\27[1B") -- Cursor down
	print()
end

-- Tool function to display downloading progress
function downloadProgress(total, current)
	if downloadComplete then return end

	local ratio = math.min(math.max(current / total, 0), 1);
	if ratio > downloadNextStep then
		downloadNextStep = downloadNextStep + 0.1
		io.write("¤")
		io.flush()
	end

	if current == total then
		downloadComplete = true
		downloadStop()
	end
end
