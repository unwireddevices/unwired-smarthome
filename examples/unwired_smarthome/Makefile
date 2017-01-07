all: ud-rpl_root ud-button

ifndef VARIANT
  ${info Not VARIANT defines! Set default: normal}
  VARIANT = normal
endif

PROJECT_SOURCEFILES += ud-dag_node.c 
DEFINES+=PROJECT_CONF_H=\"project-conf.h\"
PROJECTDIRS += $(VARIANT)
CONTIKI = ../..
CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
