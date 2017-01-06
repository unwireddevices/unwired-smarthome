#!/bin/bash

make -j5 TARGET=unwired PLATFORM=srf06/cc26xx && /Applications/ti/Uniflash/flash_cc2650.sh ud-rpl_root.hex && minicom -D /dev/cu.SLAB_USBtoUART && make -j5 clean TARGET=unwired PLATFORM=srf06/cc26xx