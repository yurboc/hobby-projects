-- Prepare Wi-Fi
cfg={}
cfg.ssid="YOUR_SSID" ---   SSID for your Wi-Fi AP here
cfg.pwd="YOUR_PWD"   ---   Password for your Wi-Fi AP here
cfg.auto=true
wifi.setmode(wifi.STATION)
wifi.sta.config(cfg)

-- Prepare WS2812 buffer
numberOfLeds = 117
bytesPerLed = 3 -- Green,Red,Blue for RGB
buffer = ws2812.newBuffer(numberOfLeds, bytesPerLed)
buffer:fill(0, 0, 0)

-- prepare WS2812 LED strip
ws2812.init()
ws2812.write(buffer)

-- prepare MQTT client
mqtt_client = mqtt.Client("mqtt_led_client", 120)
mqtt_connected = false

-- Get system info
majorVer, minorVer, devVer, chipid, flashid, flashsize, flashmode, flashspeed = node.info();
print("System Info:  ")
print("NodeMCU "..majorVer.."."..minorVer.."."..devVer.."\nFlashsize: "..flashsize.."\nChipID: "..chipid)
print("FlashID: "..flashid.."\n".."Flashmode: "..flashmode.."\nHeap: "..node.heap())

-- Get file system info
remaining, used, total=file.fsinfo()
print("\nFile system info:\nTotal : "..total.." Bytes\nUsed : "..used.." Bytes\nRemain: "..remaining.." Bytes")
print("\nReady (waiting to start server)")

-- Start HTTP server
tmr.create():alarm(10000, tmr.ALARM_SINGLE,  function() dofile("_srv.lua") end)
tmr.create():alarm(15000, tmr.ALARM_SINGLE,  function() dofile("_rtc.lua") end)
tmr.create():alarm(20000, tmr.ALARM_SINGLE,  function() dofile("_mqtt.lua") end)
