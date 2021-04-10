-- read status from file "info.lua"
fd_info = file.open(FILENAME, "r")

-- sends strings from file
local function send(localSocket)
  if fd_info then
    local info_str = fd_info:readline()
    if info_str then
      localSocket:send(info_str)
    else
      fd_info:close()
      localSocket:close()
    end
  else
    print("Wrong file to send.\n")
    localSocket:close()
  end
end

-- triggers the send() function again
CLIENT:on("sent", send)

-- first call of send()
send(CLIENT)
