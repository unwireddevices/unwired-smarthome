#!/usr/local/bin/lua

local rs232 = require("rs232") -- Последняя версия для Lua 5.2 и собираемая под винду: https://github.com/moteus/librs232
-- Там же есть файл rockspecs/rs232-0.1.0-1.rockspec, который можно подсунуть luarocks, после чего он должен все собрать автоматически: luarocks install rs232-0.1.0-1.rockspec

local socket = require("socket") -- Сокеты просто ставятся через luarocks: luarocks install luasocket

-- Адрес порта, к которому подключена плата, и адрес прошивки указывается в параметрах командной строки: ./program.lua image_file port_name
-- Переменная с адресом распложена в строке 124

--/*---------------------------------------------------------------------------*/--

local uart_cycle_permit = 1
local cmd_flash = '/Applications/ti/Uniflash/ccs_base/DebugServer/bin/DSLite flash --config /Applications/ti/Uniflash/cc1310f128.ccxml --load-settings /Applications/ti/Uniflash/generated.ufsettings --verbose --flash --verify'
local cmd_reboot = '/Applications/ti/Uniflash/ccs_base/DebugServer/bin/DSLite memory --config /Applications/ti/Uniflash/cc1310f128.ccxml --verbose --output /dev/null --range 0,1'
local redirection_command = " 2>&1"

--/*---------------------------------------------------------------------------*/--

function flash_result_parse(raw_result)
    local _, _, packet
    _, _, packet = string.find(raw_result, "(Success)")
    if (packet == nil) then
        _, _, packet = string.find(raw_result, "(Failed)")
    end
    return packet
end

--/*---------------------------------------------------------------------------*/--

function exe_command(command)
    local handle = io.popen(command)
    local result = handle:read("*a")
    handle:close()
    return result
end
--/*---------------------------------------------------------------------------*/--

function colors(color)
	if (color == "red") then
		io.write(string.char(27,91,51,49,109))
	elseif (color == "none") then
		io.write(string.char(27,91,48,109))
	end
end

--/*---------------------------------------------------------------------------*/--

raw_print = print

function print(data)
	io.write(data or "")
	io.write("\n")
	io.flush()
end

function print_n(data)
	io.write(data or "")
	io.flush()
end

function print_red(data)
	colors("red")
	print(data)
	colors("none")
end

--/*---------------------------------------------------------------------------*/--

function add_byte_to_buffer(buffer, byte)
	table.insert(buffer, byte)
	while (#buffer > 95) do
		table.remove(buffer, 1)
	end
end

function clean_buffer(buffer)
	for i = 1, #buffer do
		table.insert(buffer, 0)
	end
end

function get_buffer(buffer)
	return table.concat(buffer)
end

--/*---------------------------------------------------------------------------*/--

function uart_cycle(limit)
	local _, data_read, packet, message
	uart_cycle_permit = 1
    main_cycle_limit_reached = 0
	local buffer = {}


    e, p = rs232.open(port_name)
    if e ~= rs232.RS232_ERR_NOERROR then
        print(string.format("can't open serial port '%s', error: '%s'\n",
                port_name, rs232.error_tostring(e)))
        return
    end

    assert(p:set_baud_rate(rs232.RS232_BAUD_115200) == rs232.RS232_ERR_NOERROR)
    assert(p:set_data_bits(rs232.RS232_DATA_8) == rs232.RS232_ERR_NOERROR)
    assert(p:set_parity(rs232.RS232_PARITY_NONE) == rs232.RS232_ERR_NOERROR)
    assert(p:set_stop_bits(rs232.RS232_STOP_1) == rs232.RS232_ERR_NOERROR)
    assert(p:set_flow_control(rs232.RS232_FLOW_OFF) == rs232.RS232_ERR_NOERROR)

    if (limit ~= nil) then
		now_time = socket.gettime()*1000
		end_time = now_time+(limit*1000)
	end

 	while (uart_cycle_permit == 1 and main_cycle_limit_reached == 0) do
		_, data_read = p:read(1, 200)
		if (data_read ~= nil) then
			--print_n(data_read)
			add_byte_to_buffer(buffer, data_read)
			local buffer_state = get_buffer(buffer)
			_, _, address = string.find(buffer_state, "Link layer addr: (........................)$")
			if (address ~= nil) then
                    clean_buffer(buffer)
                    print_n("Device address: ")
					print_red(address) -- <<<<======== переменная address это и есть адрес устройства
                    uart_cycle_permit = 0
			end
		end

        if (limit ~= nil) then
			now_time = socket.gettime()*1000
			if (now_time > end_time) then
				main_cycle_limit_reached = 1
			end
		end

	end
	return main_cycle_limit_reached
end

--/*---------------------------------------------------------------------------*/--

function flash_reboot(image_file)
    print_n("Trying to flash... ")
    local status_flash = flash_result_parse(exe_command(cmd_flash..image_file..redirection_command))
    if (status_flash ~= "Success") then
        print("Flash error")
        return "Flash error"
    else
        print("Flash ok")
    end

    print_n("Trying to reboot... ")
    local reboot_flash = flash_result_parse(exe_command(cmd_reboot..redirection_command))
    if (reboot_flash ~= "Success") then
        print("Reboot error")
        return "Reboot error"
    else
        print("Reboot ok")
        return "OK"
    end

    print("Program error, please reboot programmer")
    return "ERROR"
end

--/*---------------------------------------------------------------------------*/--

if (arg[1] == nil or arg[2] == nil) then
		print("use:\tprogram.lua image_file port_name")
		return
end
local image_file = " "..arg[1]
port_name = arg[2]

while (true) do

    local status_flash_reboot = flash_reboot(image_file)
    if (status_flash_reboot == "OK") then
        local status = uart_cycle(20)
        if (status == 0) then
            print("Device is flashed and loaded successfully")
        else
            print("Device is flashed, and not boot or not correct message")
        end
    end

    print("Press Enter to continue")
    io.read()
end







