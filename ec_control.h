/*
 * Copyright (C) 2018,  Earth Computing Inc.  All rights reserved.
 *
 *
 * @file          ec_control.h
 * @brief         ENTL state machine 
 *
 * Author:        Atsushi Kasuya
 *
 */

#ifndef _EC_CONTROL_H_
#define _EC_CONTROL_H_

#ifdef __cplusplus 
extern "C" {
#endif
#include <stdint.h>

#include "atomic_link_op.h"
#include "ec_link.h"

// return value flag for entl_received
#define ENTL_ACTION_NOP             0
#define ENTL_ACTION_SEND            0x00000001
#define ENTL_ACTION_SEND_AIT        0x00000002
#define ENTL_ACTION_PROC_AIT        0x00000004
#define ENTL_ACTION_CLER_AIT        0x00000008
#define ENTL_ACTION_SIG_AIT         0x00000010
#define ENTL_ACTION_SEND_DAT        0x00000020
#define ENTL_ACTION_SIG_ERR         0x00000040
#define ENTL_ACTION_SET_ADDR        0x00000080
#define ENTL_ACTION_DROP_AIT        0x00000100
#define ENTL_ACTION_SEND_ALO        0x00000200
#define ENTL_ACTION_SEND_ALO_T      0x00000400
#define ENTL_ACTION_SEND_ALO_F      0x00000800
#define ENTL_ACTION_PROPAGATE_AIT   0x00001000
#define ENTL_ACTION_RECOVER         0x00002000
#define ENTL_ACTION_SIG_ALO         0x00004000
#define ENTL_ACTION_SIG_LUP         0x00008000
#define ENTL_ACTION_SIG_LDN         0x00010000
#define ENTL_ACTION_FORWARD         0x80000000
#define ENTL_ACTION_ERROR       -1

#define MAX_ENTT_QUEUE_SIZE 32

// ALO command decoding
#define GEN_ALO_COMMAND(sc,dt,op) ( (sc<<21) | (dt<<16) | op )
#define ALO_COMMAND_OPCODE(x)   ((x)&0xffff)
#define ALO_COMMAND_DT(x)       ((x >> 16) & 0x1f)
#define ALO_COMMAND_SC(x)       ((x >> 21) & 0x1f)
#define ALO_COMMAND_FW(x)       ( x & 0x80000000 )

#define SET_ECLP_ALO(x)         ( (((uint64_t)ALO_COMMAND_SC(x))<<42) | (((uint64_t)ALO_COMMAND_DT(x))<<37) | (((uint64_t)ALO_COMMAND_OPCODE(x)) << 16) )

// The data structre represents the internal state of ENTL
typedef volatile struct entl_state_machine {
  ec_link_reg_t  reg ;
  alo_regs_t ao ;
  uint32_t last_alo_command ;       // keep last alo command for completion
  uint64_t last_d_addr ;       // keep d_addr on AIT for further propergation
  uint64_t retry_count ;			// how many recover happened
  uint64_t entt_count ;			    // how many entt transaction happened
  uint64_t aop_count ;			    // how many aop transaction happened
#ifndef NETRONOME_CHIP
  char *name ;
#endif

} entl_state_machine_t ;


#define ETH_P_ECLP  0xEAC0    /* Earth Computing Link Protocol [ NOT AN OFFICIALLY REGISTERED ID ] */

// MAC Destination Field Extraction Masks
#define ECLP_FW_MASK        (uint64_t)0x800000000000
#define ECLP_ETYPE_MASK     (uint64_t)0x001f00000000
#define ECLP_OP_ST_MASK     (uint64_t)0x0000ffff0000
#define ECLP_VALUE_MASK     (uint64_t)0x00000000ffff

// MAC Source Field Extraction Masks
#define ECLP_D_MASK         (uint64_t)0x800000000000
#define ECLP_H_MASK         (uint64_t)0x400000000000
#define ECLP_CREDIT_MASK    (uint64_t)0x3fff00000000
#define ECLP_NLKUP_MASK     (uint64_t)0x0000ffffffff

#define GET_ECLP_ETYPE(x)     (x&ECLP_ETYPE_MASK)
#define GET_ECLP_VALUE(x)     (x&ECLP_VALUE_MASK)
#define GET_ECLP_DIRECTION(x) ((x&ECLP_D_MASK)>>47)
#define GET_ECLP_CREDIT(x)    ((x&ECLP_CREDIT_MASK)>>32)
#define GET_ECLP_NLKUP(x)     (x&ECLP_NLKUP_MASK)

/**
 * @brief Initialize the state machine register and create the first message data
 * @param mcn : pointer to the state machine structure
 * @param addr : Output of d_addr
 *
 */
void entl_state_init( __lmem entl_state_machine_t *mcn, uint64_t *addr ) ;

/**
 * @brief On Received message, this should be called with the massage (MAC source & destination addr)
 * @param mcn : pointer to the state machine structure
 * @param d_addr : MAC destination address field
 * @param s_value : source value for Atomic Link Operation
 * @param d_value : pointer to returning value for Atomic Link Operation
 * @param ait_queue : number of messages in AIT queue
 * @param ait_command : Command field on the top of AIT QUEUE
 * @param egress_queue : number of messages in Egress queue
 * @param addr : Output of d_addr
 * @param alo_data : Output of alo data 
 *
 * @return ENTL_ACTION defined above
 */
int entl_received( __lmem entl_state_machine_t *mcn, uint64_t d_addr, uint64_t s_value, uint32_t ait_queue, uint32_t ait_command, uint32_t egress_queue, uint64_t *addr, uint64_t *alo_data ) ;

/**
 * @brief On sending recover packet, this function should be called to get the destination MAC address for message
 * @param mcn : pointer to the state machine structure
 * @param addr : pointer to MAC destination adress field, returning the value to be sent
 * @return ENTL_ACTION defined above
 */
int entl_recover( __lmem entl_state_machine_t *mcn, uint64_t *addr ) ;

#ifdef __cplusplus 
}
#endif

#endif /* _EC_CONTROL_H_ */
