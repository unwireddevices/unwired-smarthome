#!/bin/bash

make -j5 TARGET=unwired BOARD=srf06/cc26xx && /Applications/ti/Uniflash/backdoor-bootloader.py -e -w -v ud-rpl_root.bin && ../../tools/sky/serialdump-macos -b115200 /dev/tty.usbserial-* && make -j5 clean TARGET=unwired BOARD=srf06/cc26xx