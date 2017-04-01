.PHONY: all clean light root root-CLEAN light-CLEAN 

all: root-CLEAN light-CLEAN

ifndef CPU
  ${info Not CPU define! USE: "CPU=cc26xx" or "CPU=cc13xx" Set default: cc26xx}
  CPU = cc26xx
endif

ifndef BOARD
  ${info Not BOARD define! USE: "BOARD=srf06" or "BOARD=udboards" Set default: udboards}
  BOARD = udboards
endif

root-CLEAN: root clean2

light-CLEAN: light clean3
	
root:
	CONF="NOLEAF"; \
	touch /tmp/unwired_last_conf; \
	[ "x`cat /tmp/unwired_last_conf`" != "x$$CONF" ] && make clean_no_hex; \
	echo $$CONF > /tmp/unwired_last_conf
	make -j5 TARGET=unwired BOARD=$(BOARD)/$(CPU) -f makefiles/Makefile.root
	arm-none-eabi-size root.elf  | grep -v text > /tmp/size.tmp 
	lua ../../tools/mem_used.lua /tmp/size.tmp 125000 20000

light:
	CONF="NOLEAF"; \
	touch /tmp/unwired_last_conf; \
	[ "x`cat /tmp/unwired_last_conf`" != "x$$CONF" ] && make clean; \
	echo $$CONF > /tmp/unwired_last_conf
	make -j5 TARGET=unwired BOARD=$(BOARD)/$(CPU) -f makefiles/Makefile.light
	arm-none-eabi-size light.elf | grep -v text > /tmp/size.tmp 
	lua ../../tools/mem_used.lua /tmp/size.tmp 125000 20000
	
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
	
light-fb: light-CLEAN
	while true; do ../../tools/backdoor-bootloader.py -e -w -v light.hex && break; done;

light-f: light-CLEAN
	/Applications/ti/Uniflash/flash_cc2650.sh light.hex

root-fb: root-CLEAN
	while true; do ../../tools/backdoor-bootloader.py -e -w -v root.hex && break; done;
	
root-f: root-CLEAN
	/Applications/ti/Uniflash/flash_cc2650.sh root.hex

serial:
	../../tools/sky/serialdump-macos -b115200 /dev/tty.usbserial-* || ../../tools/sky/serialdump-macos -b115200 /dev/tty.SLAB_USBtoUART
	
minicom:
	minicom -D /dev/tty.usbserial-* ||  minicom -D /dev/tty.SLAB_USBtoUART
	
cppcheck:
	cppcheck -q -f --enable=all -I ../../*  ./ 2>&1 |grep incotex

