.PHONY: all clean cleanall ud-button-NONCLEAN ud-rpl_root-NONCLEAN

all: ud-rpl_root-CLEAN ud-button-CLEAN

ifndef CPU
  ${info Not CPU define! USE: "CPU=cc26xx/cc13xx" Set default: cc26xx}
  CPU = cc26xx
endif

ifndef BOARD
  ${info Not BOARD define! USE: "CPU=udboards/srf06" Set default: udboards}
  BOARD = udboards
endif

ud-button-CLEAN: ud-button clean

ud-rpl_root-CLEAN: ud-rpl_root clean2

ud-button:
	make -j5 TARGET=unwired BOARD=$(BOARD)/$(CPU) -f Makefile.ud-button

ud-rpl_root:
	make -j5 TARGET=unwired BOARD=$(BOARD)/$(CPU) -f Makefile.ud-rpl_root
	
clean:
	-rm -f *~ *core core *.srec *.lst *.map *.cprg *.bin *.data contiki*.a *.firmware core-labels.S *.ihex *.ini *.ce *.co *.unwired *.elf 	

clean2:
	-rm -f *~ *core core *.srec *.lst *.map *.cprg *.bin *.data contiki*.a *.firmware core-labels.S *.ihex *.ini *.ce *.co *.unwired *.elf 	

cleanall: clean
	-rm -rf obj_unwired
	-rm -f *.hex
	
ud-button-flashb: ud-button-CLEAN
	/Applications/ti/Uniflash/backdoor-bootloader.py -e -w -v ud-button.hex

ud-rpl_root-flashb: ud-rpl_root-CLEAN
	/Applications/ti/Uniflash/backdoor-bootloader.py -e -w -v ud-rpl_root.hex

ud-button-flash: ud-button-CLEAN
	/Applications/ti/Uniflash/flash_cc2650.sh ud-button.hex
	
ud-rpl_root-flash: ud-rpl_root-CLEAN
	/Applications/ti/Uniflash/flash_cc2650.sh ud-rpl_root.hex

serial:
	../../tools/sky/serialdump-macos -b115200 /dev/tty.usbserial-*
	
minicom:
	minicom -D /dev/tty.usbserial-*
