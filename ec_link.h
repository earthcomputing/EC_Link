/*
 * Copyright (C) 2018,  Earth Computing Inc.  All rights reserved.
 *
 *
 * @file          ec_link.h
 * @brief         ENTL state machine 
 *
 * Author:        Atsushi Kasuya
 *
 */

#ifndef _EC_LINK_H_
#define _EC_LINK_H_

#ifdef __cplusplus 
extern "C" {
#endif

#ifdef NETRONOME_CHIP
  // Chip environment

#else
  // Host environment
#define ENTL_DEBUG(fmt, args...) if(debug_flag) printf( fmt, ## args )

#endif
#include <stdint.h>

#define TC_THRESHHOLD 3

#define ENTT_NIL 0
#define ENTT_Ra 1
#define ENTT_Rb 2

#define TOKEN_BACKOFF_THRESHHOLD 4
#define TOKEN_HOLD_THRESHHOLD 2

#define ENTL_ACTION_RECOVER    0x01
#define ENTL_ACTION_ENTT_STRT  0x02
#define ENTL_ACTION_ENTT_DONE  0x04
#define ENTL_ACTION_ENTT_RECV  0x08
#define ENTL_ACTION_ENTT_FNSH  0x10
#define ENTL_ACTION_ENTT_DROP  0x20

typedef struct ec_link_reg {
  union {
  	struct {
  		unsigned int sr:1 ;  // sr reg
  		unsigned int rr:1 ;  // rr reg
  		unsigned int tt:1 ;  // tt reg (toggle)
  		unsigned int tf:1 ;  // tf reg (indicate ENTL working)
      unsigned int token:1 ; // token (owned when entt is successed)
  		unsigned int tc:16 ; // tc reg (count toggle failure)
      unsigned int entt:2 ; // 0: entl, 1: Ra, 2:Rb 
      unsigned int entt_bkoff:4 ; // back off counter for entt retry
  		unsigned int dummy:5 ;
  	};
  	unsigned int _raw ;
  };
} ec_link_reg_t ;

/*

  The EC Link field is occupied in 'value' field of original ECLP header.

  _SR and RR packet should not be merged to support recovery capability. 

  1. 'SR'
     valid = 1, s_or_r = 1, value = _SR
  2. 'RR'
     valid = 1, s_or_r = 0, value = RR

  tecktack = 1: TECK is sent when s_on_0 = 1
  tecktack = 1: TACK is sent when s_on_0 = 0
  token = 1 to transfer ENTT token, token should not be passed when tecktack = 1

*/

typedef struct ec_link_field {
  union {
  	struct {
  		unsigned int valid:1 ;  // valid0
      unsigned int s_or_r:1 ;  // 1:s, 0:r
  		unsigned int value:1 ;  // value
      unsigned int tecktack:1; // TECK if s_or_r = 1, TACK if s_or_r = 0
      unsigned int token:1;    // pass token 
  		unsigned int dummy: 11 ;
  	};
  	unsigned short _raw ;
  };
} ec_link_field_t ;

/**
 * @brief Initialize the EC_Link Register
 *
 */
void ec_link_init( ec_link_reg_t *reg ) ;

/**
 * @brief Initial SR send 
 *
 */
void ec_link_send( ec_link_reg_t *reg, ec_link_field_t *ret ) ;

/**
 * @brief Update EC_Link reg according to the received packet field
 * @param reg : pointer to the EC_Link register
 * @param field : ec_link_field_t field in LSB 16bit of MAC destination address
 * @param entt : Request to transit to ENTT sequence if possible
 * @param ret : pointer to ec_link_field_t to be placed at LSB 16bit of MAC destination address 
 *
 */
int ec_link_action( ec_link_reg_t *reg, ec_link_field_t field, int entt, ec_link_field_t *ret ) ;

void ec_link_resend( ec_link_reg_t *reg, ec_link_field_t field, ec_link_field_t *ret ) ;

#ifdef __cplusplus 
}
#endif


#endif
