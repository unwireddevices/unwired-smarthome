.PHONY: all clean cleanall 

all: ud-rpl_root-CLEAN ud-button-CLEAN ud-relay-CLEAN

ifndef CPU
  ${info Not CPU define! USE: "CPU=cc26xx/cc13xx" Set default: cc26xx}
  CPU = cc26xx
endif

ifndef BOARD
  ${info Not BOARD define! USE: "BOARD=udboards/srf06" Set default: udboards}
  BOARD = udboards
endif

ud-button-CLEAN: ud-button clean

ud-rpl_root-CLEAN: ud-rpl_root clean2

ud-relay-CLEAN: ud-relay clean3


ud-button:
	make -j5 TARGET=unwired BOARD=$(BOARD)/$(CPU) -f Makefile.ud-button

ud-rpl_root:
	make -j5 TARGET=unwired BOARD=$(BOARD)/$(CPU) -f Makefile.ud-rpl_root

ud-relay:
	make -j5 TARGET=unwired BOARD=$(BOARD)/$(CPU) -f Makefile.ud-relay
	
clean:
	-rm -f *~ *core core *.srec *.lst *.map *.cprg *.bin *.data contiki*.a *.firmware core-labels.S *.ihex *.ini *.ce *.co *.unwired *.elf 	

clean2:
	-rm -f *~ *core core *.srec *.lst *.map *.cprg *.bin *.data contiki*.a *.firmware core-labels.S *.ihex *.ini *.ce *.co *.unwired *.elf 	

clean3:
	-rm -f *~ *core core *.srec *.lst *.map *.cprg *.bin *.data contiki*.a *.firmware core-labels.S *.ihex *.ini *.ce *.co *.unwired *.elf 	

cleanall: clean
	-rm -rf obj_unwired
	-rm -f *.hex
	
ud-button-flashb: ud-button-CLEAN
	/Applications/ti/Uniflash/backdoor-bootloader.py -e -w -v ud-button.hex ||  /Applications/ti/Uniflash/backdoor-bootloader.py -e -w -v ud-button.hex

ud-button-flash: ud-button-CLEAN
	/Applications/ti/Uniflash/flash_cc2650.sh ud-button.hex

ud-rpl_root-flashb: ud-rpl_root-CLEAN
	/Applications/ti/Uniflash/backdoor-bootloader.py -e -w -v ud-rpl_root.hex ||  /Applications/ti/Uniflash/backdoor-bootloader.py -e -w -v ud-rpl_root.hex
	
ud-rpl_root-flash: ud-rpl_root-CLEAN
	/Applications/ti/Uniflash/flash_cc2650.sh ud-rpl_root.hex

ud-relay-flashb: ud-relay-CLEAN
	/Applications/ti/Uniflash/backdoor-bootloader.py -e -w -v ud-relay.hex ||  /Applications/ti/Uniflash/backdoor-bootloader.py -e -w -v ud-relay.hex
	
ud-relay-flash: ud-relay-CLEAN
	/Applications/ti/Uniflash/flash_cc2650.sh ud-relay.hex

serial:
	../../tools/sky/serialdump-macos -b115200 /dev/tty.usbserial-* || ../../tools/sky/serialdump-macos -b115200 /dev/tty.SLAB_USBtoUART
	
minicom:
	minicom -D /dev/tty.usbserial-* ||  minicom -D /dev/tty.SLAB_USBtoUART
	
cppcheck:
	cppcheck -q -f --enable=all -I ../../*  ./ 2>&1 |grep unwired_smarthome

