.PHONY: all clean light root root-CLEAN light-CLEAN

all: root-CLEAN light-CLEAN

ifndef CPU
  CPU = cc13xx
  ${info Not CPU define, use: "CPU=cc26xx" or "CPU=cc13xx". Set default: $(CPU)}
endif

include ../Makefile_global_unwired

$(eval $(call generate-targets,light,NOLEAF,$(CPU),0x01A00001))
$(eval $(call generate-targets,root,NOLEAF,$(CPU),0x01FF0001))

# 0x01FF0001 — asuno-light(asuno, 01), root(FF), version 0001
# 0x01A00001 — asuno-light(asuno, 01), light(A0), version 0001
