#ifndef _UD_WORLD_H_
#define _UD_WORLD_H_
/*---------------------------------------------------------------------------*/
#include "contiki.h"
/*---------------------------------------------------------------------------*/
PROCESS_NAME(rpl_node_process);

typedef struct 
{
  uint8_t connected;
  uip_ip6addr_t *root_addr; 
} connect_info_t;
/*---------------------------------------------------------------------------*/
#endif
