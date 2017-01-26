.PHONY: all clean 

all: root-CLEAN button-CLEAN relay-CLEAN

ifndef CPU
  ${info Not CPU define! USE: "CPU=cc26xx/cc13xx" Set default: cc26xx}
  CPU = cc26xx
endif

ifndef BOARD
  ${info Not BOARD define! USE: "BOARD=udboards/srf06" Set default: udboards}
  BOARD = udboards
endif

button-CLEAN: button clean1

root-CLEAN: root clean2

relay-CLEAN: relay clean3


button:
	CONF="LEAF"; \
	touch /tmp/unwired_last_conf; \
	[ "x`cat /tmp/unwired_last_conf`" != "x$$CONF" ] && make clean; \
	echo $$CONF > /tmp/unwired_last_conf
	make -j5 TARGET=unwired BOARD=$(BOARD)/$(CPU) -f makefiles/Makefile.button

root:
	CONF="NOLEAF"; \
	touch /tmp/unwired_last_conf; \
	[ "x`cat /tmp/unwired_last_conf`" != "x$$CONF" ] && make clean; \
	echo $$CONF > /tmp/unwired_last_conf
	make -j5 TARGET=unwired BOARD=$(BOARD)/$(CPU) -f makefiles/Makefile.root

relay:
	CONF="NOLEAF"; \
	touch /tmp/unwired_last_conf; \
	[ "x`cat /tmp/unwired_last_conf`" != "x$$CONF" ] && make clean; \
	echo $$CONF > /tmp/unwired_last_conf
	make -j5 TARGET=unwired BOARD=$(BOARD)/$(CPU) -f makefiles/Makefile.relay
	
clean1:
	-rm -f *~ *core core *.srec *.lst *.map *.cprg *.bin *.data contiki*.a *.firmware core-labels.S *.ihex *.ini *.ce *.co *.unwired *.elf 	

clean2:
	-rm -f *~ *core core *.srec *.lst *.map *.cprg *.bin *.data contiki*.a *.firmware core-labels.S *.ihex *.ini *.ce *.co *.unwired *.elf 	

clean3:
	-rm -f *~ *core core *.srec *.lst *.map *.cprg *.bin *.data contiki*.a *.firmware core-labels.S *.ihex *.ini *.ce *.co *.unwired *.elf 	

clean: 
	-rm -f *~ *core core *.srec *.lst *.map *.cprg *.bin *.data contiki*.a *.firmware core-labels.S *.ihex *.ini *.ce *.co *.unwired *.elf 	
	-rm -rf obj_unwired
	-rm -f *.hex
	
button-flashb: button-CLEAN
	/Applications/ti/Uniflash/backdoor-bootloader.py -e -w -v button.hex ||  /Applications/ti/Uniflash/backdoor-bootloader.py -e -w -v button.hex

button-flash: button-CLEAN
	/Applications/ti/Uniflash/flash_cc2650.sh button.hex

root-flashb: root-CLEAN
	/Applications/ti/Uniflash/backdoor-bootloader.py -e -w -v root.hex ||  /Applications/ti/Uniflash/backdoor-bootloader.py -e -w -v root.hex
	
root-flash: root-CLEAN
	/Applications/ti/Uniflash/flash_cc2650.sh root.hex

relay-flashb: relay-CLEAN
	/Applications/ti/Uniflash/backdoor-bootloader.py -e -w -v relay.hex ||  /Applications/ti/Uniflash/backdoor-bootloader.py -e -w -v relay.hex
	
relay-flash: relay-CLEAN
	/Applications/ti/Uniflash/flash_cc2650.sh relay.hex

serial:
	../../tools/sky/serialdump-macos -b115200 /dev/tty.usbserial-* || ../../tools/sky/serialdump-macos -b115200 /dev/tty.SLAB_USBtoUART
	
minicom:
	minicom -D /dev/tty.usbserial-* ||  minicom -D /dev/tty.SLAB_USBtoUART
	
cppcheck:
	cppcheck -q -f --enable=all -I ../../*  ./ 2>&1 |grep unwired_smarthome

