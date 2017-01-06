#!/bin/bash

make -j5 TARGET=unwired BOARD=srf06/cc26xx && /Applications/ti/Uniflash/flash_cc2650.sh ud-rpl_root.hex && ../../tools/sky/serialdump-macos -b115200 /dev/tty.usbserial-* && make -j5 clean TARGET=unwired BOARD=srf06/cc26xx