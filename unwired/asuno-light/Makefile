.PHONY: all clean light root root-CLEAN light-CLEAN

all: root-CLEAN light-CLEAN

ifndef CPU
  ${info Not CPU define! USE: "CPU=cc26xx" or "CPU=cc13xx" Set default: cc13xx}
  CPU = cc13xx
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
	arm-none-eabi-size root.elf  | grep -v text | lua ../../tools/mem_used.lua 125000 20000 || echo "------ Lua not found ------"
	../bootloader/generate-metadata/generate-metadata root.bin 0x0 0x01FF0001 1
	srec_cat firmware-metadata.bin -binary root.bin -binary -offset 0x100 -o root-ota-image.bin -binary
	srec_cat ../bootloader/bootloader.hex -intel -crop 0x0 0x2000 0x1FFA8 0x20000 root-ota-image.bin -binary -offset 0x2000 -crop 0x2000 0x1B000 -o root-firmware.hex -intel
	rm root.hex

light: net-bootloader
	CONF="NOLEAF"; \
	touch /tmp/unwired_last_conf; \
	[ "x`cat /tmp/unwired_last_conf`" != "x$$CONF" ] && make clean; \
	echo $$CONF > /tmp/unwired_last_conf
	make -j5 TARGET=unwired BOARD=$(BOARD)/$(CPU) -f makefiles/Makefile.light
	arm-none-eabi-size light.elf | grep -v text | lua ../../tools/mem_used.lua 125000 20000 || echo "------ Lua not found ------"
	../bootloader/generate-metadata/generate-metadata light.bin 0x0 0x01A00001 1
	srec_cat firmware-metadata.bin -binary light.bin -binary -offset 0x100 -o light-ota-image.bin -binary
	srec_cat ../bootloader/bootloader.hex -intel -crop 0x0 0x2000 0x1FFA8 0x20000 light-ota-image.bin -binary -offset 0x2000 -crop 0x2000 0x1B000 -o light-firmware.hex -intel
	rm light.hex

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

net-bootloader-clean:
	cd ../bootloader && make clean

net-bootloader:
	cd ../bootloader && make -j5 CPU=$(CPU) bootloader.hex

light-fb: light-CLEAN
	while true; do ../../tools/backdoor-bootloader.py -e -w -v light-firmware.hex && break; done;

light-f: light-CLEAN
	/Applications/ti/Uniflash/flash_cc1310.sh light-firmware.hex && \
	osascript -e 'display notification with title "Flash asuno/light" subtitle "Success"' || \
	osascript -e 'display notification with title "Flash asuno/light" subtitle "FAIL"' || echo "osascript not found"

light-ota: light
	scp light-ota-image.bin root@192.168.20.187:/root/unwired_smarthome


root-fb: root-CLEAN
	while true; do ../../tools/backdoor-bootloader.py -e -w -v root-firmware.hex && break; done;

root-f: root-CLEAN
	/Applications/ti/Uniflash/flash_cc1310.sh root-firmware.hex && \
	osascript -e 'display notification with title "Flash asuno/root" subtitle "Success"' || \
	osascript -e 'display notification with title "Flash asuno/root" subtitle "FAIL"' || echo "osascript not found"

serial:
	../../tools/sky/serialdump-macos -b115200 /dev/tty.usbserial-* || ../../tools/sky/serialdump-macos -b115200 /dev/tty.SLAB_USBtoUART

minicom:
	minicom -D /dev/tty.usbserial-* ||  minicom -D /dev/tty.SLAB_USBtoUART

