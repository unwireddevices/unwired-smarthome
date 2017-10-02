math.round = function(num, idp)
  local mult = 10^(idp or 0)
  return math.floor(num * mult + 0.5) / mult
end

local max_flash = tonumber(arg[1]) or 125000
local max_ram = tonumber(arg[2]) or 20000

local result = io.stdin:read("*a")
local _, _, text, data, bss = string.find(result, '%s*(%d*)%s*(%d*)%s*(%d*)')
text = tonumber(text)
data = tonumber(data)
bss = tonumber(bss)
if (text ~= nil and data ~= nil and bss ~= nil) then
	local flash = text+data
	local ram = data+bss
	local flash_kb = math.round((flash/1024), 1)
	local ram_kb = math.round((ram/1024), 1)
	local flash_percent = math.round((flash/(max_flash/100)), 1)
	local ram_percent = math.round((ram/(max_ram/100)), 1)
	print("/----------------------------------------------------------------------------------------------/")
	print("Program used FLASH: "..flash_kb.."kb("..flash_percent.."%), RAM: "..ram_kb.."kb("..ram_percent.."%)")
	print("/----------------------------------------------------------------------------------------------/\n\n")
end
