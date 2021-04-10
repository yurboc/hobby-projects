print("Start server")

-- create a server
srv = net.createServer(net.TCP)

-- server listen on port 80
srv:listen(80, function(conn)
  conn:on("receive", function(client, request)

    -- use wifi tools
    if string.sub(request, 0, 11) == "**command**" then
      payload = request
      dofile("_tools.lua")

      client:close()
      collectgarbage()
      return
    end

    -- extract request parameters
    local _, _, method, path, vars = string.find(request, "([A-Z]+) (.+)?(.+) HTTP")
    if method == nil then
      _, _, method, path = string.find(request, "([A-Z]+) (.+) HTTP")
    end
    
    -- parse request parameters
    _GET = {}
    if vars ~= nil then
      for k, v in string.gmatch(vars, "(%w+)=(%w+)&*") do
        _GET[k] = v
      end
    end

    -- parse request
    if next(_GET) ~= nil then

      buf = ""
      outfile = ""
      dofile("_get.lua")
      if outfile ~= "" then
        -- send FILE as response
        CLIENT = client
        FILENAME = outfile
        dofile("_stream.lua")
      else
        -- send TEXT as response
        client:send(buf)
        client:close()
      end
      
    else
    
      -- write status to file "info.lua"
      print("Getting status...\n")
      dofile("_stat.lua")
      tmr.delay(250)
      
      -- send "info.lua" as stream
      CLIENT = client
      FILENAME = "_info.lua"
      dofile("_stream.lua")

    end

    collectgarbage()
  end)
end)

print("Server running...")
