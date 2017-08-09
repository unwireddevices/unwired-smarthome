local bindechex = require("bindechex")


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
device_ability[DEVICE_ABILITY_BUTTON] = "Button/switch"

DEVICE_ABILITY_TEMPERATURE     =     "02"
device_ability[DEVICE_ABILITY_TEMPERATURE] = "Temperature sensor"

DEVICE_ABILITY_HUMIDITY        =     "03"
device_ability[DEVICE_ABILITY_HUMIDITY] = "Humidity sensor"

DEVICE_ABILITY_PRESSURE        =     "04"
device_ability[DEVICE_ABILITY_PRESSURE] = "Pressure sensor"

DEVICE_ABILITY_LIGHT_SENSOR    =     "05"
device_ability[DEVICE_ABILITY_LIGHT_SENSOR] = "Light sensor"

DEVICE_ABILITY_NOISE_SENSOR    =     "06"
device_ability[DEVICE_ABILITY_NOISE_SENSOR] = "Noise sensor"

DEVICE_ABILITY_MOTION_SENSOR   =     "07"
device_ability[DEVICE_ABILITY_MOTION_SENSOR] = "Motion sensor"

DEVICE_ABILITY_RESERVED1       =     "08"

DEVICE_ABILITY_C02_SENSOR      =     "09"
device_ability[DEVICE_ABILITY_C02_SENSOR] = "CO2 sensor"

DEVICE_ABILITY_CO_SENSOR       =     "0A"
device_ability[DEVICE_ABILITY_CO_SENSOR] = "CO sensor"

DEVICE_ABILITY_GAS_SENSOR      =     "0B"
device_ability[DEVICE_ABILITY_GAS_SENSOR] = "GAS sensor"

DEVICE_ABILITY_POWER_METER     =     "0C"
device_ability[DEVICE_ABILITY_POWER_METER] = "Power/voltage meter"

DEVICE_ABILITY_RADIATION_METER       =     "0D"
device_ability[DEVICE_ABILITY_RADIATION_METER] = "Radiation meter"

DEVICE_ABILITY_RESERVED3       =     "0E"
DEVICE_ABILITY_RESERVED4       =     "0F"
DEVICE_ABILITY_RESERVED5       =     "10"

DEVICE_ABILITY_RELAY           =     "11"
device_ability[DEVICE_ABILITY_RELAY] = "Relay"


DEVICE_ABILITY_DIMMER          =     "12"
device_ability[DEVICE_ABILITY_DIMMER] = "Dimmer"

DEVICE_ABILITY_RESERVED6       =     "13"
DEVICE_ABILITY_RESERVED7       =     "14"
DEVICE_ABILITY_RESERVED8       =     "15"
DEVICE_ABILITY_RESERVED9       =     "16"
DEVICE_ABILITY_RESERVED10      =     "17"

DEVICE_ABILITY_LED             =     "18"
device_ability[DEVICE_ABILITY_LED] = "LED indicator"

-----------------------------------------------------------------------------------


-----------------------------------------------------------------------------------
device_button_events = {}

DEVICE_ABILITY_BUTTON_EVENT_CLICK        =   "01"
device_button_events[DEVICE_ABILITY_BUTTON_EVENT_CLICK] = "click"

DEVICE_ABILITY_BUTTON_EVENT_LONG_CLICK   =   "02"
device_button_events[DEVICE_ABILITY_BUTTON_EVENT_LONG_CLICK] = "longclick"

DEVICE_ABILITY_BUTTON_EVENT_ON           =   "03"
device_button_events[DEVICE_ABILITY_BUTTON_EVENT_ON] = "on"

DEVICE_ABILITY_BUTTON_EVENT_OFF          =   "04"
device_button_events[DEVICE_ABILITY_BUTTON_EVENT_OFF] = "off"
-----------------------------------------------------------------------------------
device_relay_commands = {}

DEVICE_ABILITY_RELAY_COMMAND_ON        =   "01"
device_relay_commands[DEVICE_ABILITY_RELAY_COMMAND_ON] = "on"

DEVICE_ABILITY_RELAY_COMMAND_OFF   =   "00"
device_relay_commands[DEVICE_ABILITY_RELAY_COMMAND_OFF] = "off"

DEVICE_ABILITY_RELAY_COMMAND_TOGGLE   =   "80"
device_relay_commands[DEVICE_ABILITY_RELAY_COMMAND_TOGGLE] = "toggle"
-----------------------------------------------------------------------------------

device_sleep_type = {}

DEVICE_SLEEP_TYPE_NORMAL             =           "01"
device_sleep_type[DEVICE_SLEEP_TYPE_NORMAL] = "Non-sleep"

DEVICE_SLEEP_TYPE_LEAF               =           "02"
device_sleep_type[DEVICE_SLEEP_TYPE_LEAF] = "Leaf mode"
-----------------------------------------------------------------------------------


PROTOCOL_VERSION_V1            =     "01"
DEVICE_VERSION_V1              =     "01"

UART_PROTOCOL_VERSION_V1       =     "01"
UART_PROTOCOL_VERSION_V2       =     "02"

DATA_TYPE_JOIN                            =              "01" --Запрос на включение в сеть
DATA_TYPE_SENSOR_DATA                     =              "02" --Данные с датчиков устройства
DATA_TYPE_CONFIRM                 	      =              "03" --Подтверждение запроса на включение в сеть
DATA_TYPE_PONG                            =              "04" --Подтверждение доставки пакета
DATA_TYPE_COMMAND                         =              "05" --Команды возможностям устройства
DATA_TYPE_STATUS                          =              "06" --Пакет со статусными данными
DATA_TYPE_GET_STATUS                      =              "07" --Запрос статуса(не реализовано)
DATA_TYPE_SETTINGS                        =              "08" --Команда настройки параметров
DATA_TYPE_WARNING                         =              "09" --Ошибки и предупреждения(не реализовано)
DATA_TYPE_SET_TIME                        =              "0A" --Команда установки времени(не реализовано)
DATA_TYPE_SET_SCHEDULE                    =              "0B" --Команда установки расписания(не реализовано)
DATA_TYPE_FIRMWARE                        =              "0C" --Команда с данными для OTA
DATA_TYPE_UART                            =              "0D" --Команда с данными UART


function packet_processing(a, data)
	local ipv6_adress = a[1]..a[2]..":"..a[3]..a[4]..":"..a[5]..a[6]..":"..a[7]..a[8]..":"..a[9]..a[10]..":"..a[11]..a[12]..":"..a[13]..a[14]..":"..a[15]..a[16]
	if (data.p_version == PROTOCOL_VERSION_V1 and data.dev_version == DEVICE_VERSION_V1) then
		if data.d_type == DATA_TYPE_JOIN then
		 	print("PPM: Join packet from "..ipv6_adress)
		 	--join_data_processing(ipv6_adress, data)

		elseif data.d_type == DATA_TYPE_SENSOR_DATA then
			print("PPM: Data from sensor")
			--sensor_data_processing(ipv6_adress, data)

		elseif data.d_type == DATA_TYPE_CONFIRM then
			print("PPM: Data type: DATA_TYPE_CONFIRM from "..ipv6_adress)

		elseif data.d_type == DATA_TYPE_PING then
			print("PPM: Data type: DATA_TYPE_PING from "..ipv6_adress)

		elseif data.d_type == DATA_TYPE_COMMAND then
			print("PPM: Data type: DATA_TYPE_COMMAND from "..ipv6_adress)

		elseif data.d_type == DATA_TYPE_STATUS then
			print("PPM: Data type: DATA_TYPE_STATUS from "..ipv6_adress)
			--status_data_processing(ipv6_adress, data)

		else
			print(string.format("Unsupported protocol type(%s)", tostring(data.d_type)))

		end
	else
		print(string.format("Unsupported protocol version(%s) or device version(%s)", tostring(data.p_version), tostring(data.dev_version)))
		print("Adress: "..ipv6_adress)
	end
end


function packet_parse(packet)
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




packet = arg[1]
if (packet ~= nil) then
	packet_parse(packet)
end





