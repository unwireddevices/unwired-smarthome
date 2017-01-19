local rs232 = require("luars232")
local socket = require("socket")
local socket = require "socket"
local bindechex = require("bindechex")

port_name = "/dev/ttyATH0"

device_group = {}
DEVICE_GROUP_BUTTON_SWITCH     =     "00"
device_group[DEVICE_GROUP_BUTTON_SWITCH]		   =     "Button/switch"

DEVICE_GROUP_SENSORS           =     "01"
device_group[DEVICE_GROUP_SENSORS]				   =     "Sensor"

DEVICE_GROUP_MOTION_SENSOR     =     "02"
device_group[DEVICE_GROUP_MOTION_SENSOR]		   =     "Motion sensor"

DEVICE_GROUP_OPEN_SENSORS      =     "03"
device_group[DEVICE_GROUP_OPEN_SENSORS]			   =     "Door open sensor"

DEVICE_GROUP_METERS            =     "04"
device_group[DEVICE_GROUP_METERS]				   =     "Meter"

DEVICE_GROUP_RELAY             =     "05"
device_group[DEVICE_GROUP_RELAY]				   =     "Relay"

DEVICE_GROUP_DIMMER            =     "06"
device_group[DEVICE_GROUP_DIMMER]				   =     "Dimmer"

DEVICE_GROUP_LIGHT             =     "07"
device_group[DEVICE_GROUP_LIGHT]				   =     "Light"

DEVICE_GROUP_RGB_LIGHT         =     "08"
device_group[DEVICE_GROUP_RGB_LIGHT]			   =     "RGB light"

DEVICE_GROUP_BRIDGE_CONVERTER  =     "09"
device_group[DEVICE_GROUP_BRIDGE_CONVERTER]		   =     "Bridge/Converter"

DEVICE_GROUP_OTHER             =     "FF"
device_group[DEVICE_GROUP_OTHER]				   =     "Other device"

-----------------------------------------------------------------------------------


-----------------------------------------------------------------------------------
device_ability = {}

DEVICE_ABILITY_NONE            =     "00"

DEVICE_ABILITY_BUTTON          =     "01"
device_ability[DEVICE_ABILITY_BUTTON] 				= "Button/switch"

DEVICE_ABILITY_TEMPERATURE     =     "02"
device_ability[DEVICE_ABILITY_TEMPERATURE] 			= "Temperature sensor"

DEVICE_ABILITY_HUMIDITY        =     "03"
device_ability[DEVICE_ABILITY_HUMIDITY] 			= "Humidity sensor"

DEVICE_ABILITY_PRESSURE        =     "04"
device_ability[DEVICE_ABILITY_PRESSURE] 			= "Pressure sensor"

DEVICE_ABILITY_LIGHT_SENSOR    =     "05"
device_ability[DEVICE_ABILITY_LIGHT_SENSOR] 		= "Light sensor"

DEVICE_ABILITY_NOISE_SENSOR    =     "06"
device_ability[DEVICE_ABILITY_NOISE_SENSOR] 		= "Noise sensor"

DEVICE_ABILITY_MOTION_SENSOR   =     "07"
device_ability[DEVICE_ABILITY_MOTION_SENSOR] 		= "Motion sensor"

DEVICE_ABILITY_RESERVED1       =     "08"

DEVICE_ABILITY_C02_SENSOR      =     "09"
device_ability[DEVICE_ABILITY_C02_SENSOR] 			= "CO2 sensor"

DEVICE_ABILITY_CO_SENSOR       =     "0A"
device_ability[DEVICE_ABILITY_CO_SENSOR] 			= "CO sensor"

DEVICE_ABILITY_GAS_SENSOR      =     "0B"
device_ability[DEVICE_ABILITY_GAS_SENSOR] 			= "GAS sensor"

DEVICE_ABILITY_POWER_METER     =     "0C"
device_ability[DEVICE_ABILITY_POWER_METER]	 		= "Power/voltage meter"

DEVICE_ABILITY_RESERVED2       =     "0D"
DEVICE_ABILITY_RESERVED3       =     "0E"
DEVICE_ABILITY_RESERVED4       =     "0F"
DEVICE_ABILITY_RESERVED5       =     "10"

DEVICE_ABILITY_RELAY           =     "11"
device_ability[DEVICE_ABILITY_RELAY] 				= "Relay"


DEVICE_ABILITY_DIMMER          =     "12"
device_ability[DEVICE_ABILITY_DIMMER] 				= "Dimmer"

DEVICE_ABILITY_RESERVED6       =     "13"
DEVICE_ABILITY_RESERVED7       =     "14"
DEVICE_ABILITY_RESERVED8       =     "15"
DEVICE_ABILITY_RESERVED9       =     "16"
DEVICE_ABILITY_RESERVED10      =     "17"

DEVICE_ABILITY_LED             =     "18"
device_ability[DEVICE_ABILITY_LED] 					= "LED indicator"

-----------------------------------------------------------------------------------


-----------------------------------------------------------------------------------
device_button_events = {}

DEVICE_ABILITY_BUTTON_EVENT_CLICK        =   "01"
device_button_events[DEVICE_ABILITY_BUTTON_EVENT_CLICK] 		= "click"

DEVICE_ABILITY_BUTTON_EVENT_LONG_CLICK   =   "02"
device_button_events[DEVICE_ABILITY_BUTTON_EVENT_LONG_CLICK] 	= "longclick"

DEVICE_ABILITY_BUTTON_EVENT_ON           =   "03"
device_button_events[DEVICE_ABILITY_BUTTON_EVENT_ON] 			= "on"

DEVICE_ABILITY_BUTTON_EVENT_OFF          =   "04"
device_button_events[DEVICE_ABILITY_BUTTON_EVENT_OFF] 			= "off"
-----------------------------------------------------------------------------------
device_relay_commands = {}

DEVICE_ABILITY_RELAY_COMMAND_ON        =   "01"
device_relay_commands[DEVICE_ABILITY_RELAY_COMMAND_ON] 			= "on"

DEVICE_ABILITY_RELAY_COMMAND_OFF   =   "00"
device_relay_commands[DEVICE_ABILITY_RELAY_COMMAND_OFF] 		= "off"

DEVICE_ABILITY_RELAY_COMMAND_TOGGLE   =   "02"
device_relay_commands[DEVICE_ABILITY_RELAY_COMMAND_TOGGLE] 		= "toggle"
-----------------------------------------------------------------------------------

device_sleep_type = {}

DEVICE_SLEEP_TYPE_NORMAL             =           "01"
device_sleep_type[DEVICE_SLEEP_TYPE_NORMAL] 					= "Non-sleep"

DEVICE_SLEEP_TYPE_LEAF               =           "02"
device_sleep_type[DEVICE_SLEEP_TYPE_LEAF] 						= "Leaf mode"
-----------------------------------------------------------------------------------


PROTOCOL_VERSION_V1            =     "01"
DEVICE_VERSION_V1              =     "01"
UART_PROTOCOL_VERSION_V1       =     "01"



UART_PV1_START_MQ = "011616161610"
UART_PV1_STOP_MQ = "031616161704"
UART_NONE_DATA = "FFFFFFFFFFFFFFFFFFFF"

VOLTAGE_PRESCALER = 16

-----------------------------------------------------------------------------------
DATA_TYPE_JOIN                 =     "01"
DATA_TYPE_SENSOR_DATA          =     "02"
DATA_TYPE_CONFIRM              =     "03"
DATA_TYPE_PING                 =     "04"
DATA_TYPE_COMMAND              =     "05"
DATA_TYPE_STATUS               =     "06"



local start_time = 0


function string.fromhex(str)
    local str = string.gsub(str, " ", "") 
    return (str:gsub('..', function (cc)
        return string.char(tonumber(cc, 16))
    end))
end

function string.tohex(str)
    return (str:gsub('.', function (c)
        return string.format('%02X ', string.byte(c))
    end))
end

function led(state)
	state = "noled"
	if (state == "on") then
		os.execute("echo 1 > /sys/devices/platform/leds-gpio/leds/unwone\:green\:eth1_rxtx/brightness")
	elseif (state == "off") then
		os.execute("echo 0 > /sys/devices/platform/leds-gpio/leds/unwone\:green\:eth1_rxtx/brightness")
	end
end

function ipv6_adress_parse(ipv6_adress)
	local adress_capturing = "(%w%w)(%w%w):(%w%w)(%w%w):(%w%w)(%w%w):(%w%w)(%w%w):(%w%w)(%w%w):(%w%w)(%w%w):(%w%w)(%w%w):(%w%w)(%w%w)"
	local _
	local a = {}

	_, end_1, a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8],a[9],a[10],a[11],a[12],a[13],a[14],a[15],a[16]  = string.find(ipv6_adress, adress_capturing)
	if (end_1 ~= nil) then
		return a[1]..a[2]..a[3]..a[4]..a[5]..a[6]..a[7]..a[8]..a[9]..a[10]..a[11]..a[12]..a[13]..a[14]..a[15]..a[16]
	else
		print("IV6P: Adress parse error"..ipv6_adress)
		return nil
	end
end

function send_command_to_ability(ipv6_adress, ability_target, ability_number, ability_state)
	local adress = ipv6_adress_parse(ipv6_adress)
	p:write(UART_NONE_DATA:fromhex())
	socket.sleep(0.001)
	p:write(UART_PV1_START_MQ:fromhex())
	socket.sleep(0.001)
	p:write(PROTOCOL_VERSION_V1:fromhex())
	socket.sleep(0.001)
	p:write(adress:fromhex())
	socket.sleep(0.001)
	p:write(ability_target:fromhex())
	socket.sleep(0.001)
	p:write(ability_number:fromhex())
	socket.sleep(0.001)
	p:write(ability_state:fromhex())
	socket.sleep(0.001)
	p:write(UART_NONE_DATA:fromhex())
	socket.sleep(0.001)
	p:write(UART_PV1_STOP_MQ:fromhex())
	socket.sleep(0.001)

	
	print("Processing time "..(math.ceil(socket.gettime()*1000 - start_time)).." ms")
	--print(raw_data:tohex())
end


function send_relay_command(ipv6_adress, relay_number, state)
	--print("Relay command processing start: +"..(socket.gettime()*1000 - start_time).." ms")
	local ability_state, ability

	if (state == device_relay_commands[DEVICE_ABILITY_RELAY_COMMAND_ON]) then
		ability_state = DEVICE_ABILITY_RELAY_COMMAND_ON
	elseif (state == device_relay_commands[DEVICE_ABILITY_RELAY_COMMAND_OFF]) then
		ability_state = DEVICE_ABILITY_RELAY_COMMAND_OFF
	elseif (state == device_relay_commands[DEVICE_ABILITY_RELAY_COMMAND_TOGGLE]) then
		ability_state = DEVICE_ABILITY_RELAY_COMMAND_TOGGLE
	end

	if (relay_number == 1) then
		ability_number = tostring("01")
	elseif (state == 2) then
		ability_number = tostring("02")
	end


	
	if (ability_state ~= nil) then
		send_command_to_ability(ipv6_adress, DEVICE_ABILITY_RELAY, ability_number, ability_state)
	else
		print("Non-valid ability state!")
	end
end


function sensor_data_processing(ipv6_adress, data)
	--print("Sensor data processing module")
	--print("Sensor data processing start: +"..(socket.gettime()*1000 - start_time).." ms")
	local number_ability = data.b1 or "no number_ability"
	local sensor_number = data.b3 or "no sensor_number"
	local sensor_event = data.b4 or "no sensor_event"

	local sensor_name = device_ability[number_ability] or "Not found ability description: "..number_ability

	print("SDPM: Adress: "..ipv6_adress)
	print("SDPM: Sensor type: "..sensor_name)
	if (number_ability == DEVICE_ABILITY_BUTTON) then
		button_name = string.upper(tostring(sensor_number):fromhex())
		print(" BDPM: Button name: "..button_name)
		print(" BDPM: Button event: "..device_button_events[sensor_event])


		
		if (button_name == "A") then
			ipv6_adress = "fe80:0000:0000:0000:0212:4b00:0939:301f"
			send_relay_command(ipv6_adress, 1, "toggle")
		end
		if (button_name == "B") then
			ipv6_adress = "fe80:0000:0000:0000:0212:4b00:0939:3661"
			send_relay_command(ipv6_adress, 1, "toggle")
		end
		if (button_name == "C") then
			ipv6_adress = "fe80:0000:0000:0000:0212:4b00:0a8b:5142"
			send_relay_command(ipv6_adress, 1, "toggle")
		end
		if (button_name == "D") then
			if (device_button_events[sensor_event] == "click") then
				ipv6_adress = "fe80:0000:0000:0000:0212:4b00:0939:301f"
				send_relay_command(ipv6_adress, 1, "on")
				socket.sleep(0.1) 
				ipv6_adress = "fe80:0000:0000:0000:0212:4b00:0939:3661"
				send_relay_command(ipv6_adress, 1, "on")
				socket.sleep(0.1) 
				ipv6_adress = "fe80:0000:0000:0000:0212:4b00:0a8b:5142"
				send_relay_command(ipv6_adress, 1, "on")
			elseif (device_button_events[sensor_event] == "longclick") then
				ipv6_adress = "fe80:0000:0000:0000:0212:4b00:0939:301f"
				send_relay_command(ipv6_adress, 1, "off")
				socket.sleep(0.1) 
				ipv6_adress = "fe80:0000:0000:0000:0212:4b00:0939:3661"
				send_relay_command(ipv6_adress, 1, "off")
				socket.sleep(0.1) 
				ipv6_adress = "fe80:0000:0000:0000:0212:4b00:0a8b:5142"
				send_relay_command(ipv6_adress, 1, "off")
			end
		end
	end
end

function join_data_processing(ipv6_adress, data)
	--print("Join data processing module")
	local current_device_group = data.b1 or "no device_group"
	local current_sleep_type = data.b2 or "no sleep_type"
	local ability_1 = data.b3 or "no ability_1"
	local ability_2 = data.b4 or "no ability_2"
	local ability_3 = data.b5 or "no ability_3"
	local ability_4 = data.b6 or "no ability_4"

	local device_group_name = device_group[current_device_group] or current_device_group
	local device_sleep_name = device_sleep_type[current_sleep_type] or current_sleep_type
	print("JDPM: Join packet from "..ipv6_adress..", device group: "..device_group_name..", sleep type: "..device_sleep_name)
end

function getu16(data, pos) -- reads a 16 bit value of the string data at position pos
	local high, low = string.byte(data, pos, pos+1)
	local val = high*256 + low -- no bit fiddling in lua since everything is a double internally
	return val
end

function status_data_processing(ipv6_adress, data)
	--print("Join status processing module")
	local ipv6_adress_parent_short = data.b1..data.b2..":"..data.b3..data.b4..":"..data.b5..data.b6..":"..data.b7..data.b8

	local uptime = tonumber(bindechex.Hex2Dec((data.b12 or 00)..(data.b11 or 00)..(data.b10 or 00)..(data.b9 or 00)))
	local voltage = (bindechex.Hex2Dec(data.b16 or 00)*VOLTAGE_PRESCALER/1000)
	local temp = bindechex.Hex2Dec(data.b15 or 00)
	local parent_rssi_raw = tonumber(bindechex.Hex2Dec((data.b14 or 00)..(data.b13 or 00)))
	if parent_rssi_raw > 32768 then
		parent_rssi_raw = parent_rssi_raw - 65536
	end

	parent_rssi = string.format("%d, %i, %u", parent_rssi_raw, parent_rssi_raw, parent_rssi_raw) or "no rssi"
	parent_rssi = parent_rssi_raw
	print("SDPM: Status packet from "..ipv6_adress..":")
	print(" Parent adress: "..ipv6_adress_parent_short)
	print(" Uptime: "..uptime.."s")
	print(" Parent RSSI: "..parent_rssi.."dbm")
	print(" Temp: "..temp.."C")
	print(" Voltage: "..voltage.." v")
end


function packet_processing(a, data)
	--print("Packet processing module")
	--print("Packet processing start: +"..(socket.gettime()*1000 - start_time).." ms")
	local ipv6_adress = a[1]..a[2]..":"..a[3]..a[4]..":"..a[5]..a[6]..":"..a[7]..a[8]..":"..a[9]..a[10]..":"..a[11]..a[12]..":"..a[13]..a[14]..":"..a[15]..a[16]
	if (data.p_version == PROTOCOL_VERSION_V1 and data.dev_version == DEVICE_VERSION_V1) then
		if data.d_type == DATA_TYPE_JOIN then
		 	--print("PPM: Join packet from "..ipv6_adress)
		 	join_data_processing(ipv6_adress, data)

		elseif data.d_type == DATA_TYPE_SENSOR_DATA then
			--print("PPM: Data from sensor")
			sensor_data_processing(ipv6_adress, data)

		elseif data.d_type == DATA_TYPE_CONFIRM then
			print("PPM: Data type: DATA_TYPE_CONFIRM from "..ipv6_adress)

		elseif data.d_type == DATA_TYPE_PING then
			print("PPM: Data type: DATA_TYPE_PING from "..ipv6_adress)

		elseif data.d_type == DATA_TYPE_COMMAND then
			print("PPM: Data type: DATA_TYPE_COMMAND from "..ipv6_adress)

		elseif data.d_type == DATA_TYPE_STATUS then
			--print("PPM: Data type: DATA_TYPE_STATUS from "..ipv6_adress)
			status_data_processing(ipv6_adress, data)

		end
	else
		print(string.format("Unsupported protocol version(%s) or device version(%s)", tostring(data.p_version), tostring(data.dev_version)))
		print("Adress: "..ipv6_adress)
	end
end


function packet_parse(packet)
	--print("Packet parse module")
	--print("Packet parse start: +"..(socket.gettime()*1000 - start_time).." ms")
	local adress_capturing_all = "(%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w)"
	local data_capturing_all = "(%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w)"
	local _, _, adress, raw_data = string.find(packet, "DAGROOTRAW1"..adress_capturing_all..data_capturing_all.."RAWEND")
	if (adress ~= nil and raw_data ~= nil) then
		local _ = {}
		local a = {}
		local d = {}
		local adress_capturing = "(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)"
		local data_capturing = "(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)(%w%w)"
		
		_, end_1, a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8],a[9],a[10],a[11],a[12],a[13],a[14],a[15],a[16]  = string.find(adress, adress_capturing)
		_, end_2, d.p_version,d.dev_version,d.d_type,d.b1,d.b2,d.b3,d.b4,d.b5,d.b6,d.b7,d.b8,d.b9,d.b10,d.b11,d.b12,d.b13,d.b14,d.b15,d.b16,d.b17,d.b18,d.b19,d.b20 = string.find(raw_data, data_capturing)
		if (end_1 ~= nil and end_2 ~= nil) then
			packet_processing(a, d)
		else
			print(string.format("Not parse adress(%s) or data(%s) in packet"), tostring(end_1), tostring(end_2))
			print(packet)
			print(adress)
			print(adress_capturing)
			print(raw_data)
			print(data_capturing)
		end
	else
		print("Not parse packet: "..packet)
	end
end


function console_print(data)
	io.write(data)
	io.flush()
end



------------------------------------------------------
local version = "0.20"
local uart_version = UART_PROTOCOL_VERSION_V1

print("RPL-router version "..version..", uart protocol version: "..uart_version.."\n")

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
assert(p:set_flow_control(rs232.RS232_FLOW_OFF)  == rs232.RS232_ERR_NOERROR)

while true do
	local err, data_read, size = p:read(1)
	assert(e == rs232.RS232_ERR_NOERROR)
	if (data_read ~= nil) then
		console_print(data_read)
		if (data_read == "D") then
			socket.sleep(0.01)
			local err, data_read, size = p:read(10)
			assert(e == rs232.RS232_ERR_NOERROR)
			if (data_read == "AGROOTRAW1") then
				start_time = socket.gettime()*1000
				socket.sleep(0.05) 
				local err, data_read, size = p:read(95-10-1)
				assert(e == rs232.RS232_ERR_NOERROR)
				if (data_read ~= nil) then
					led("on")
					console_print("\n/------------------------------------------------------/\n")
					console_print("DAGROOTRAW1"..data_read.."\n")
					packet_parse("DAGROOTRAW1"..data_read)
					led("off")
				end
			else
				console_print(data_read)
			end	
		end

	end
end
 





