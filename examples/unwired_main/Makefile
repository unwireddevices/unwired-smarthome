CONTIKI_PROJECT = ud-world
all: $(CONTIKI_PROJECT)
#APPS += 
PROJECT_SOURCEFILES += udp-leds.c udp-button.c
PROJECT_SOURCEFILES += cetic-6lbr-client.c udp-gpio-input.c 
PROJECT_SOURCEFILES += udp-sht21.c udp-lps331.c 
PROJECT_SOURCEFILES += udp-common.c 

DEFINES+=PROJECT_CONF_H=\"project-conf.h\"

CONTIKI = ../..
CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
