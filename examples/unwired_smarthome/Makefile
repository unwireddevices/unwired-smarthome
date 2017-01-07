.PHONY: all clean cleanall ud-button-NONCLEAN ud-rpl_root-NONCLEAN

all: ud-button-CLEAN ud-rpl_root-CLEAN

ifndef CPU
  ${info Not CPU define! USE: "CPU=cc26xx/cc13xx" Set default: cc26xx}
  CPU = cc26xx
endif

ifndef BOARD
  ${info Not BOARD define! USE: "CPU=udboards/srf06" Set default: udboards}
  BOARD = udboards
endif

ud-button-CLEAN: ud-button clean

ud-rpl_root-CLEAN: ud-rpl_root clean

ud-button:
	make -j5 TARGET=unwired BOARD=$(BOARD)/$(CPU) -f Makefile.ud-button

ud-rpl_root:
	make -j5 TARGET=unwired BOARD=$(BOARD)/$(CPU) -f Makefile.ud-rpl_root
	
clean:
	-rm -f *~ *core core *.srec *.lst *.map *.cprg *.data contiki*.a *.firmware core-labels.S *.ihex *.ini *.ce *.co *.unwired *.elf 	

cleanall: clean
	-rm -rf obj_unwired
	-rm -f *.hex *.bin
	
