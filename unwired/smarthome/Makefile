.PHONY: all clean dimmer relay root button root-CLEAN button-CLEAN relay-CLEAN dimmer-CLEAN

all: root-CLEAN relay-CLEAN dimmer-CLEAN button-CLEAN

ifndef CPU
  CPU = cc26xx
  ${info Not CPU define, use: "CPU=cc26xx" or "CPU=cc13xx". Set default: $(CPU)}
endif

$(eval $(call generate-targets,root,NOLEAF,$(CPU),0x00FF0001))
$(eval $(call generate-targets,button,LEAF,$(CPU),0x00A10001))
$(eval $(call generate-targets,relay,NOLEAF,$(CPU),0x00A20001))
$(eval $(call generate-targets,dimmer,NOLEAF,$(CPU),0x00A30001))
$(eval $(call generate-targets,motionsensor,NOLEAF,$(CPU),0x00A40001))

# 0x00FF0001 — unwired smarthome(00), root(FF), version 0001
# 0x00A10001 — unwired smarthome(00), button(A1), version 0001
# 0x00A20001 — unwired smarthome(00), relay(A2), version 0001
# 0x00A30001 — unwired smarthome(00), dimmer(A3), version 0001
# 0x00A40001 — unwired smarthome(00), motionsensor(A4), version 0001
