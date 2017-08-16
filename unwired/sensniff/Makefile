.PHONY: all clean sensniff sensniff-CLEAN 

all: sensniff-CLEAN

ifndef CPU
  ${info Not CPU define! USE: "CPU=cc26xx" or "CPU=cc13xx" Set default: cc13xx}
  CPU = cc13xx
endif

ifndef BOARD
  ${info Not BOARD define! USE: "BOARD=srf06" or "BOARD=udboards" Set default: udboards}
  BOARD = udboards
endif


sensniff-CLEAN: sensniff clean3

sensniff: net-bootloader
	make -j5 TARGET=unwired BOARD=$(BOARD)/$(CPU) -f Makefile.sensniff
	arm-none-eabi-size sensniff.elf | grep -v text > /tmp/size.tmp 
	lua ../../tools/mem_used.lua /tmp/size.tmp 125000 20000
	../bootloader/generate-metadata/generate-metadata sensniff.bin 0x0 0xabcd1234 1
	srec_cat firmware-metadata.bin -binary sensniff.bin -binary -offset 0x100 -o sensniff-ota-image.bin -binary
	srec_cat ../bootloader/bootloader.hex -intel -crop 0x0 0x2000 0x1FFA8 0x20000 sensniff-ota-image.bin -binary -offset 0x2000 -crop 0x2000 0x1B000 -o sensniff-firmware.hex -intel
	rm sensniff.hex
	
	
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
	
net-bootloader: 
	cd ../bootloader && make -j5 CPU=$(CPU) bootloader.hex
	
sensniff-fb: sensniff-CLEAN
	while true; do ../../tools/backdoor-bootloader.py -e -w -v sensniff-firmware.hex && break; done;

sensniff-f: sensniff-CLEAN
	/Applications/ti/Uniflash/flash_cc2650.sh sensniff-firmware.hex

serial:
	../../tools/sky/serialdump-macos -b115200 /dev/tty.usbserial-* || ../../tools/sky/serialdump-macos -b115200 /dev/tty.SLAB_USBtoUART
	
minicom:
	minicom -D /dev/tty.usbserial-* ||  minicom -D /dev/tty.SLAB_USBtoUART
	