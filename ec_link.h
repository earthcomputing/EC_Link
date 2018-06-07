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
#define ENTT_Rp 3

#define FIELD_ENTL 0
#define FIELD_ENTT 1
#define FIELD_ENTA 2

#define TOKEN_BACKOFF_THRESHHOLD 4
#define TOKEN_HOLD_THRESHHOLD 2

#define ENTL_ACTION_ENTL_RECOVER    0x0001
#define ENTL_ACTION_ENTT_STRT       0x0002
#define ENTL_ACTION_ENTT_NEXT       0x0004
#define ENTL_ACTION_ENTT_DONE       0x0008
#define ENTL_ACTION_ENTT_RECV       0x0010
#define ENTL_ACTION_ENTT_FNSH       0x0020
#define ENTL_ACTION_ENTT_DROP       0x0040
#define ENTL_ACTION_ENTT_CLER       0x0080
#define ENTL_ACTION_ENTL_LKUP       0x0100
#define ENTL_ACTION_ENTL_LKDN       0x0200
#define ENTL_ACTION_ENTL_EROR       0x8000


typedef struct ec_link_reg {
  union {
  	struct {
  		unsigned int sr:1 ;  // sr reg
  		unsigned int rr:1 ;  // rr reg
  		unsigned int tt:1 ;  // tt reg (toggle)
  		unsigned int tf:1 ;  // tf reg (indicate ENTL working)
      unsigned int token:1 ; // token (owned when entt is successed)
  		unsigned int tc:16 ; // tc reg (count toggle failure)
      unsigned int state:2 ; // 0: entl, 1: Ra, 2:Rb, 3: Rp
      unsigned int entt_bkoff:4 ; // back off counter for entt retry
      unsigned int token_bkoff:4 ; // back off counter for token hold
  		unsigned int dummy:1 ;
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
      unsigned int dummy: 9 ;
      unsigned int recover:1;
      unsigned int token:1;    // pass token 
      unsigned int entt:2;     // ENTT request, reply := TECK if s_or_r = 1, TACK if s_or_r = 0
      unsigned int value:1 ;   // value
      unsigned int s_or_r:1 ;  // 1:s, 0:r
  		unsigned int valid:1 ;   // valid0
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
 * @param qfull : ENTT queue is full, so reject any more request
 * @param ret : pointer to ec_link_field_t to be placed at LSB 16bit of MAC destination address 
 *
 */
int ec_link_action( ec_link_reg_t *reg, ec_link_field_t field, int entt, int qfull, ec_link_field_t *ret ) ;

/**
 * @brief Generate recover field on TC error detection
 * @param reg : pointer to the EC_Link register
 * @param ret : pointer to ec_link_field_t to be placed at LSB 16bit of MAC destination address 
 *
 */
int ec_link_recover( ec_link_reg_t *reg, ec_link_field_t *ret ) ;

#ifdef __cplusplus 
}
#endif


#endif
