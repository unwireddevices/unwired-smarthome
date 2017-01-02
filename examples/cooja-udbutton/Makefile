CONTIKI_PROJECT = ud-main
all: $(CONTIKI_PROJECT)
#APPS += 
PROJECT_SOURCEFILES += ud-dag_node.c ud-button.c

DEFINES+=PROJECT_CONF_H=\"project-conf.h\"

CONTIKI = ../..
CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
