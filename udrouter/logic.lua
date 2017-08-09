logic = {}

--/*---------------------------------------------------------------------------*/--

function all_relay(command)
	if (command == "off") then
		send_relay_command(relay_main_room, 1, "off")
		send_relay_command(relay_main_room_table, 1, "off")
		send_relay_command(relay_main_room_table, 2, "off")
		send_relay_command(relay_kitchen, 1, "off")
		send_relay_command(relay_hall, 1, "off")
		send_relay_command(relay_bathroom, 1, "off")
		send_relay_command(relay_wc, 1, "off")

	elseif (command == "on") then
		send_relay_command(relay_main_room, 1, "on")
		send_relay_command(relay_main_room_table, 1, "on")
		send_relay_command(relay_main_room_table, 2, "on")
		send_relay_command(relay_kitchen, 1, "on")
		send_relay_command(relay_hall, 1, "on")
		send_relay_command(relay_bathroom, 1, "on")
		send_relay_command(relay_wc, 1, "on")
	end
end


--/*---------------------------------------------------------------------------*/--

function logic.motion_sensor_event_handler(ipv6_adress, sensor_number, current_event)

	if (current_event == "motion") then
		send_relay_command(relay_hall, 1, "on")
	elseif (current_event == "nomotion") then
		--send_relay_command(relay_hall, 1, "off")
	end
end

--/*---------------------------------------------------------------------------*/--

function logic.button_sensor_event_handler(current_switch, button_name, current_event)

	if (current_switch == switch_mini_bed or current_switch == switch_mini_table) then
		if (button_name == "B" and current_event == "click") then
			all_relay("on")
			state = "all_on"

		elseif (button_name == "C" and current_event == "click") then
			if (state == "all_on") then
				send_relay_command(relay_main_room_table, 2, "off")
				send_relay_command(relay_main_room_table, 1, "off")
				state = "all_off"
			elseif (state == "all_off") then
				send_relay_command(relay_main_room_table, 2, "on")
				send_relay_command(relay_main_room_table, 1, "off")
				state = "2_on"
			elseif (state == "2_on") then
				send_relay_command(relay_main_room_table, 2, "off")
				send_relay_command(relay_main_room_table, 1, "on")
				state = "1_on"
			elseif (state == "1_on") then
				send_relay_command(relay_main_room_table, 2, "on")
				send_relay_command(relay_main_room_table, 1, "on")
				state = "all_on"
			end

		elseif (button_name == "C" and current_event == "longclick") then
			send_relay_command(relay_main_room, 1, "toggle")
		elseif (button_name == "D" and current_event == "click") then
			all_relay("off")
			state = "all_off"
		end

	elseif (current_switch == switch_mini_door) then
		if (button_name == "B" and current_event == "click") then
			all_relay("on")
			state = "all_on"
		elseif (button_name == "C" and current_event == "click") then
			send_relay_command(relay_hall, 1, "toggle")
		elseif (button_name == "D" and current_event == "click") then
			all_relay("off")
			state = "all_off"

			send_relay_command(relay_hall, 1, "on")
			socket.sleep(10)
			send_relay_command(relay_hall, 1, "off")
		end

	elseif (current_switch == switch_wc) then
		if (button_name == "A" and current_event == "click") then
			send_relay_command(relay_bathroom, 1, "toggle")

		elseif (button_name == "B" and current_event == "click") then
			send_relay_command(relay_wc, 1, "toggle")

		elseif (button_name == "C" and current_event == "click") then
			send_relay_command(relay_hall, 1, "toggle")

		elseif (button_name == "D" and current_event == "click") then
			send_relay_command(relay_kitchen, 1, "toggle")
		end

	elseif (current_switch == switch_main_room) then
		if (button_name == "A" and current_event == "click") then
			send_relay_command(relay_main_room, 1, "toggle")
		elseif (button_name == "A" and current_event == "longclick") then
			send_relay_command(relay_main_room, 1, "toggle")
		elseif (button_name == "B" and current_event == "click") then
			send_relay_command(relay_main_room, 1, "toggle")
		elseif (button_name == "B" and current_event == "longclick") then
			send_relay_command(relay_main_room, 1, "toggle")
		elseif (button_name == "C" and current_event == "click") then
			send_relay_command(relay_main_room, 1, "toggle")
		elseif (button_name == "C" and current_event == "longclick") then
			send_relay_command(relay_main_room, 1, "toggle")
		elseif (button_name == "D" and current_event == "click") then
			send_relay_command(relay_main_room, 1, "toggle")
		elseif (button_name == "D" and current_event == "longclick") then
			send_relay_command(relay_main_room, 1, "toggle")
		end

	elseif (current_switch == switch_kitchen) then
		if (button_name == "A" and current_event == "click") then
			send_relay_command(relay_kitchen, 1, "toggle")
		elseif (button_name == "A" and current_event == "longclick") then
			send_relay_command(relay_kitchen, 1, "toggle")
		elseif (button_name == "B" and current_event == "click") then
			send_relay_command(relay_kitchen, 1, "toggle")
		elseif (button_name == "B" and current_event == "longclick") then
			send_relay_command(relay_kitchen, 1, "toggle")
		elseif (button_name == "C" and current_event == "click") then
			send_relay_command(relay_kitchen, 1, "toggle")
		elseif (button_name == "C" and current_event == "longclick") then
			send_relay_command(relay_kitchen, 1, "toggle")
		elseif (button_name == "D" and current_event == "click") then
			send_relay_command(relay_kitchen, 1, "toggle")
		elseif (button_name == "D" and current_event == "longclick") then
			send_relay_command(relay_kitchen, 1, "toggle")
		end

	end
end

--/*---------------------------------------------------------------------------*/--

return logic  