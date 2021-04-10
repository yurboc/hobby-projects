-- this files gets informaton the saves it to a file call info.lua
-- info.lua is then read to conn:send

local start_time = "(unknown)"
local fd = file.open("_time.lua", "r")
if fd then
  start_time = fd:readline()
  fd:close(); fd = nil
end

file.open("_info.lua","w+")
local w = file.writeline

w("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n")

w("<html>")
w("<body bgcolor='#E6E6E6'>")

w("<h3>System Info</h3><ul>")
w("<li>IP: ")
if wifi.sta.getip() ~= nil then
  w(wifi.sta.getip())
else
  w("not available")
end
w("<li>MAC: "..wifi.sta.getmac())
local majorVer, minorVer, devVer, chipid, flashid, flashsize, flashmode, flashspeed = node.info();
w("<li>NodeMCU: "..majorVer.."."..minorVer.."."..devVer.."<li>Flashsize: "..flashsize.."<li>ChipID: "..chipid)
w("<li>FlashID: "..flashid.."<li>".."Flashmode: "..flashmode.."<li>Heap: "..node.heap().."</ul>")

w("<h3>File System</h3>")
local r, u, t = file.fsinfo() 
w("<ul><li>Total Memory: "..t.." bytes<li>Bytes Used: "..u.." bytes<li>Bytes Remain: "..r.." bytes</ul>")

w("<h3>Files in memory</h3>")
w("<table cellpadding ='2'>")
  l = file.list();
  for k,v in pairs(l) do
    w("<tr>")
    w("<td><B>"..k.."</td><td>"..v.." bytes</td>")
    w("</tr>")
  end
w("</table>")

w("<h3>System Time</h3>")

local uptime = tmr.time()
local uptime_d = uptime / (60*60*24)
local uptime_h = (uptime - uptime_d*60*60*24) / (60*60)
local uptime_m = (uptime - uptime_d*60*60*24 - uptime_h*60*60) / 60
local uptime_s = (uptime - uptime_d*60*60*24 - uptime_h*60*60 - uptime_m*60)
w(string.format("<p>Uptime: %d days, %02d:%02d:%02d</p>", uptime_d, uptime_h, uptime_m, uptime_s))

w("<p>"..start_time.."</p>")

local sec, usec = rtctime.get()
sec = sec + 3*60*60 -- UTC+3
tm = rtctime.epoch2cal(sec, usec)
w(string.format("<p>Now: %04d/%02d/%02d %02d:%02d:%02d (UTC+3)</p>", tm["year"], tm["mon"], tm["day"], tm["hour"], tm["min"], tm["sec"]))

w("<h3>LED state</h3>")
for led_id=1,buffer:size() do
  local g, r, b = buffer:get(led_id)
  local led_state = r + g + b
  local led_symbol = ":"
  if led_state == 0 then
    led_symbol = "."
  elseif led_state == 3*255 then
    led_symbol = "|"
  end
  file.write(string.format("<font color='%02x%02x%02x'>%s</font>", r, g, b, led_symbol))
end

w("<br/>")
w("<br/>")
w("<p>End of info</p>")
w("</html>")
file.close()
