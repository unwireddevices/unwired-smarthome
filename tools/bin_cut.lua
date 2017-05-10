function data_cut(all_data, segment_len)
  local max_len = segment_len
  local data_len = string.len(all_data)
  local i = 1
  local data = {}
  repeat 
    data_len = string.len(all_data)
    local data_sub = string.sub(all_data, 0, max_len)
    all_data = string.sub(all_data, max_len+1)
    data[i] = data_sub
    i = i + 1
  until (data_len < max_len+1)
  return data
end


local file = arg[1]
local path = arg[2]
local chank_size = tonumber(arg[3]) or 224

if (file == nil) then
	print("No file name!")
	return
elseif (path == nil) then
	print("No dir name!")
	return
end

local handle,err = io.open(file,"r")
if (err ~= nil) then
	print("Error open file")
	return
end
local bin_data = handle:read("*a")
handle:close()

local table_segments = data_cut(bin_data, chank_size)

for i = 1, #table_segments do 
	local filename = path.."/".."chank".."_"..i
	local handle,err = io.open(filename,"w")
	if (err ~= nil) then
		print("Error write chanks")
		return
	end
	handle:write(table_segments[i])
	handle:close()
end

