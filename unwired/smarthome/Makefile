.PHONY: all clean dimmer relay root button root-CLEAN button-CLEAN relay-CLEAN dimmer-CLEAN

all: root-CLEAN relay-CLEAN dimmer-CLEAN button-CLEAN

ifndef CPU
  ${info Not CPU define! USE: "CPU=cc26xx" or "CPU=cc13xx" Set default: cc26xx}
  CPU = cc26xx
endif

ifndef BOARD
  ${info Not BOARD define! USE: "BOARD=srf06" or "BOARD=udboards" Set default: udboards}
  BOARD = udboards
endif

button-CLEAN: button clean1

root-CLEAN: root clean2

relay-CLEAN: relay clean3

dimmer-CLEAN: dimmer clean4

button: net-bootloader
	CONF="LEAF"; \
	touch /tmp/unwired_last_conf; \
	[ "x`cat /tmp/unwired_last_conf`" != "x$$CONF" ] && make clean_no_hex; \
	echo $$CONF > /tmp/unwired_last_conf
	make -j5 TARGET=unwired BOARD=$(BOARD)/$(CPU) -f makefiles/Makefile.button
	arm-none-eabi-size button.elf | grep -v text > /tmp/size.tmp 
	lua ../../tools/mem_used.lua /tmp/size.tmp 125000 20000
	../bootloader/common/generate-metadata button.bin 0x0 0xabcd1234 1
	srec_cat firmware-metadata.bin -binary button.bin -binary -offset 0x100 -o button-ota-image.bin -binary
	srec_cat ../bootloader/bootloader.hex -intel -crop 0x0 0x2000 0x1FFA8 0x20000 button-ota-image.bin -binary -offset 0x2000 -crop 0x2000 0x1B000 -o button-firmware.hex -intel
	rm button.hex
	
	
root: net-bootloader
	CONF="NOLEAF"; \
	touch /tmp/unwired_last_conf; \
	[ "x`cat /tmp/unwired_last_conf`" != "x$$CONF" ] && make clean_no_hex; \
	echo $$CONF > /tmp/unwired_last_conf
	make -j5 TARGET=unwired BOARD=$(BOARD)/$(CPU) -f makefiles/Makefile.root
	arm-none-eabi-size root.elf  | grep -v text > /tmp/size.tmp 
	lua ../../tools/mem_used.lua /tmp/size.tmp 125000 20000
	../bootloader/common/generate-metadata root.bin 0x0 0xabcd1234 1
	srec_cat firmware-metadata.bin -binary root.bin -binary -offset 0x100 -o root-ota-image.bin -binary
	srec_cat ../bootloader/bootloader.hex -intel -crop 0x0 0x2000 0x1FFA8 0x20000 root-ota-image.bin -binary -offset 0x2000 -crop 0x2000 0x1B000 -o root-firmware.hex -intel
	rm root.hex

relay: net-bootloader
	CONF="NOLEAF"; \
	touch /tmp/unwired_last_conf; \
	[ "x`cat /tmp/unwired_last_conf`" != "x$$CONF" ] && make clean_no_hex; \
	echo $$CONF > /tmp/unwired_last_conf
	make -j5 TARGET=unwired BOARD=$(BOARD)/$(CPU) -f makefiles/Makefile.relay
	arm-none-eabi-size relay.elf | grep -v text > /tmp/size.tmp 
	lua ../../tools/mem_used.lua /tmp/size.tmp 125000 20000
	../bootloader/common/generate-metadata relay.bin 0x0 0xabcd1234 1
	srec_cat firmware-metadata.bin -binary relay.bin -binary -offset 0x100 -o relay-ota-image.bin -binary
	srec_cat ../bootloader/bootloader.hex -intel -crop 0x0 0x2000 0x1FFA8 0x20000 relay-ota-image.bin -binary -offset 0x2000 -crop 0x2000 0x1B000 -o relay-firmware.hex -intel
	rm relay.hex
	
dimmer: net-bootloader
	CONF="NOLEAF"; \
	touch /tmp/unwired_last_conf; \
	[ "x`cat /tmp/unwired_last_conf`" != "x$$CONF" ] && make clean; \
	echo $$CONF > /tmp/unwired_last_conf
	make -j5 TARGET=unwired BOARD=$(BOARD)/$(CPU) -f makefiles/Makefile.dimmer
	arm-none-eabi-size dimmer.elf | grep -v text > /tmp/size.tmp 
	lua ../../tools/mem_used.lua /tmp/size.tmp 125000 20000
	../bootloader/common/generate-metadata dimmer.bin 0x0 0xabcd1234 1
	srec_cat firmware-metadata.bin -binary dimmer.bin -binary -offset 0x100 -o dimmer-ota-image.bin -binary
	srec_cat ../bootloader/bootloader.hex -intel -crop 0x0 0x2000 0x1FFA8 0x20000 dimmer-ota-image.bin -binary -offset 0x2000 -crop 0x2000 0x1B000 -o dimmer-firmware.hex -intel
	rm dimmer.hex

net-bootloader: 
	cd ../bootloader && make -j5 CPU=$(CPU) bootloader.hex
	
clean1:
	-rm -f *~ *core core *.srec *.lst *.map *.cprg *.bin *.data contiki*.a *.firmware core-labels.S *.ihex *.ini *.ce *.co *.unwired *.elf *.o *.d	

clean2:
	-rm -f *~ *core core *.srec *.lst *.map *.cprg *.bin *.data contiki*.a *.firmware core-labels.S *.ihex *.ini *.ce *.co *.unwired *.elf  *.o *.d		

clean3:
	-rm -f *~ *core core *.srec *.lst *.map *.cprg *.bin *.data contiki*.a *.firmware core-labels.S *.ihex *.ini *.ce *.co *.unwired *.elf  *.o *.d		

clean4:
	-rm -f *~ *core core *.srec *.lst *.map *.cprg *.bin *.data contiki*.a *.firmware core-labels.S *.ihex *.ini *.ce *.co *.unwired *.elf  *.o *.d		

clean_no_hex:
	-rm -f *~ *core core *.srec *.lst *.map *.cprg *.bin *.data contiki*.a *.firmware core-labels.S *.ihex *.ini *.ce *.co *.unwired *.elf  *.o *.d		
	-rm -rf obj_unwired

clean: 
	-rm -f *~ *core core *.srec *.lst *.map *.cprg *.bin *.data contiki*.a *.firmware core-labels.S *.ihex *.ini *.ce *.co *.unwired *.elf  *.o *.d		
	-rm -rf obj_unwired
	-rm -f *.hex
	
button-fb: button-CLEAN
	while true; do ../../tools/backdoor-bootloader.py -e -w -v button-firmware.hex && break; done;

button-f: button-CLEAN
	/Applications/ti/Uniflash/flash_cc2650.sh button-firmware.hex

root-fb: root-CLEAN
	while true; do ../../tools/backdoor-bootloader.py -e -w -v root-firmware.hex && break; done;
	
root-f: root-CLEAN
	/Applications/ti/Uniflash/flash_cc2650.sh root-firmware.hex

relay-fb: relay-CLEAN
	while true; do ../../tools/backdoor-bootloader.py -e -w -v relay-firmware.hex && break; done;
	
relay-f: relay-CLEAN
	/Applications/ti/Uniflash/flash_cc2650.sh relay-firmware.hex
	
dimmer-fb: dimmer-CLEAN
	while true; do ../../tools/backdoor-bootloader.py -e -w -v dimmer-firmware.hex && break; done;
	
dimmer-f: dimmer-CLEAN
	/Applications/ti/Uniflash/flash_cc2650.sh dimmer-firmware.hex

serial:
	../../tools/sky/serialdump-macos -b115200 /dev/tty.usbserial-* || ../../tools/sky/serialdump-macos -b115200 /dev/tty.SLAB_USBtoUART
	
minicom:
	minicom -D /dev/tty.usbserial-* ||  minicom -D /dev/tty.SLAB_USBtoUART
	
cppcheck:
	cppcheck -q -f --enable=all -I ../../*  ./ 2>&1 |grep unwired_smarthome

