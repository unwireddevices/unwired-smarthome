#!/bin/bash

make -j5 TARGET=unwired BOARD=srf06/cc26xx && /Applications/ti/Uniflash/flash_cc2650.sh ud-button.hex && minicom -D /dev/cu.SLAB_USBtoUART && make -j5 clean TARGET=unwired BOARD=srf06/cc26xx