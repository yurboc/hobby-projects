-- Get RTC time
print('Getting RTC time...')
sntp_sync_done = false
sntp.sync("192.168.1.103", -- use RPi as NTP server
  function(sec, usec, server, info)
    print('RTC sync done!', sec, usec, server, info)
    if sntp_sync_done then 
      return
    end
    file.open("_time.lua","w")
    sec = sec + 3*60*60 -- UTC+3
    tm = rtctime.epoch2cal(sec, usec)
    file.writeline(string.format("RTC sync: %04d/%02d/%02d %02d:%02d:%02d (UTC+3)", tm["year"], tm["mon"], tm["day"], tm["hour"], tm["min"], tm["sec"]))
    file.close()
    sntp_sync_done = true
  end,
  function(errcode, info)
    print('RTC sync failed!', errcode, info)
    return
  end,
  1
)
