/*
 * Copyright (c) 2016, Unwired Devices LLC. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
/*---------------------------------------------------------------------------*/
/**
 * \file
 *         Header file with ommon functions for UDP services.  
 * \author
 *         Mikhail Churikov mc@unwds.com
 */
/*---------------------------------------------------------------------------*/
#ifndef UDP_COMMON_H_
#define UDP_COMMON_H_
/*---------------------------------------------------------------------------*/
#include "contiki.h"
/*---------------------------------------------------------------------------*/
typedef struct {
  uip_ipaddr_t *ip_addr;
  uint8_t leds_status;
  uint8_t connected;
  uint8_t pwm_num;
  uint8_t pwm_duty;
} status_reply_data_t;
/*---------------------------------------------------------------------------*/
typedef enum {
  GET = 'g',
  SET = 's',
  TOGGLE = 't'
} incoming_command_t;
/*---------------------------------------------------------------------------*/
int send_udp_to_mote(struct uip_udp_conn *server_conn, uip_ipaddr_t *ip_addr, 
                      uint32_t port, char *data);
/*---------------------------------------------------------------------------*/
int open_udp_connection(struct uip_udp_conn *server_conn, uint32_t port);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#endif /* UDP_COMMON_H_ */
