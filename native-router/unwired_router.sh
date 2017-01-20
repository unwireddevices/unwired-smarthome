#!/bin/sh /etc/rc.common

START=10 
STOP=15

start() {   
	cd /root/unwired_smarthome/
	lua router.lua
}                 

stop() {          
	kill `cat /tmp/run/unwired_router.pid`
}