.PHONY: all clean dimmer root root-CLEAN dimmer-CLEAN

all: root-CLEAN light-CLEAN

ifndef CPU
  ${info Not CPU define! USE: "CPU=cc26xx/cc13xx" Set default: cc26xx}
  CPU = cc26xx
endif

ifndef BOARD
  ${info Not BOARD define! USE: "BOARD=udboards/srf06" Set default: udboards}
  BOARD = udboards
endif


root-CLEAN: root clean1

light-CLEAN: light clean2

root:
	CONF="NOLEAF"; \
	touch /tmp/unwired_last_conf; \
	[ "x`cat /tmp/unwired_last_conf`" != "x$$CONF" ] && make clean; \
	echo $$CONF > /tmp/unwired_last_conf
	make -j5 TARGET=unwired BOARD=$(BOARD)/$(CPU) -f makefiles/Makefile.root

light:
	CONF="NOLEAF"; \
	touch /tmp/unwired_last_conf; \
	[ "x`cat /tmp/unwired_last_conf`" != "x$$CONF" ] && make clean; \
	echo $$CONF > /tmp/unwired_last_conf
	make -j5 TARGET=unwired BOARD=$(BOARD)/$(CPU) -f makefiles/Makefile.light
    
	
clean1:
	-rm -f *~ *core core *.srec *.lst *.map *.cprg *.bin *.data contiki*.a *.firmware core-labels.S *.ihex *.ini *.ce *.co *.unwired *.elf 	

clean2:
	-rm -f *~ *core core *.srec *.lst *.map *.cprg *.bin *.data contiki*.a *.firmware core-labels.S *.ihex *.ini *.ce *.co *.unwired *.elf 	

clean3:
	-rm -f *~ *core core *.srec *.lst *.map *.cprg *.bin *.data contiki*.a *.firmware core-labels.S *.ihex *.ini *.ce *.co *.unwired *.elf 	

clean4:
	-rm -f *~ *core core *.srec *.lst *.map *.cprg *.bin *.data contiki*.a *.firmware core-labels.S *.ihex *.ini *.ce *.co *.unwired *.elf 	


clean: 
	-rm -f *~ *core core *.srec *.lst *.map *.cprg *.bin *.data contiki*.a *.firmware core-labels.S *.ihex *.ini *.ce *.co *.unwired *.elf 	
	-rm -rf obj_unwired
	-rm -f *.hex
	
root-fb: root-CLEAN
	/Applications/ti/Uniflash/backdoor-bootloader.py -e -w -v root.hex || /Applications/ti/Uniflash/backdoor-bootloader.py -e -w -v root.hex
	
root-f: root-CLEAN
	/Applications/ti/Uniflash/flash_cc2650.sh root.hex

light-fb: light-CLEAN
	/Applications/ti/Uniflash/backdoor-bootloader.py -e -w -v light.hex || /Applications/ti/Uniflash/backdoor-bootloader.py -e -w -v light.hex
	
light-f: light-CLEAN
	/Applications/ti/Uniflash/flash_cc2650.sh light.hex

serial:
	../../tools/sky/serialdump-macos -b115200 /dev/tty.usbserial-* || ../../tools/sky/serialdump-macos -b115200 /dev/tty.SLAB_USBtoUART
	
minicom:
	minicom -D /dev/tty.usbserial-* ||  minicom -D /dev/tty.SLAB_USBtoUART
	