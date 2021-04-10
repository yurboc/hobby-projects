local r = 255
local g = 255
local b = 255
local d = 100

if _GET.color ~= nil then
  r = tonumber(string.sub(_GET.color, 1, 2), 16)
  g = tonumber(string.sub(_GET.color, 3, 4), 16)
  b = tonumber(string.sub(_GET.color, 5, 6), 16)
end

if _GET.delay ~= nil then
  d = tonumber(_GET.delay)
end

if _GET.anim == "sr" then
  local i = 1
  local shift_tmr = tmr.create()
  print("Timer 'shift_tmr' created")
  shift_tmr:alarm(d, 1, function()
    buffer:set(i, g, r, b)
    ws2812.write(buffer)
    i = i + 1
    if i > buffer:size() then
      shift_tmr:unregister()
      print("Timer 'shift_tmr' unregistered")
    end
  end)
end

if _GET.anim == "sl" then
  local i = buffer:size()
  local shift_tmr = tmr.create()
  print("Timer 'shift_tmr' created")
  shift_tmr:alarm(d, 1, function()
    buffer:set(i, g, r, b)
    ws2812.write(buffer)
    i = i - 1
    if i <= 0 then
      shift_tmr:unregister()
      print("Timer 'shift_tmr' unregistered")
    end
  end)
end
