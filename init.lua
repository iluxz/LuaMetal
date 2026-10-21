-- LuaMetal init.lua
-- this file is loaded from disk by the kernel at boot and run as the
-- OS's first program. edit it, rebuild the ESP, reboot. that's it.

print("hello from init.lua!")
print("LuaMetal is running your code from disk")

for i = 1, 5 do
  print("count " .. i)
end

print("2 + 2 = " .. 2 + 2)

-- little demo of the standard libs that are available
local t = {3, 1, 2}
table.sort(t)
print("sorted: " .. table.concat(t, ", "))

print("os.time works: " .. type(os.time))
print("string.rep works: " .. string.rep("lua", 3))

print("init.lua done")
