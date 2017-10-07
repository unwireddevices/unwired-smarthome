--[[
/*
 * Copyright (c) 2016, Unwired Devices LLC - http://www.unwireddevices.com/
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Unwired Devices nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */ ]]--
 
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
