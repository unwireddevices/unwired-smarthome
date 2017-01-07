all: ud-button ud-rpl_root

ud-button:
	make -j5 TARGET=unwired -f Makefile.ud-button

ud-rpl_root:
	make -j5 TARGET=unwired -f Makefile.ud-rpl_root 
	
clean:
	-rm -f *~ *core core *.srec *.lst *.map *.cprg *.data contiki*.a *.firmware core-labels.S *.ihex *.ini *.ce *.co *.unwired *.elf 	

cleanall: clean
	-rm -rf obj_unwired