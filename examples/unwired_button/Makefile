CONTIKI_PROJECT = ud-world
all: $(CONTIKI_PROJECT)
#APPS += 
PROJECT_SOURCEFILES += udp-common.c cetic-6lbr-client.c udp-button.c

DEFINES+=PROJECT_CONF_H=\"project-conf.h\"

CONTIKI = ../..
CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
