/*
 * Copyright (C) 2017,  Earth Computing Inc.  All rights reserved.
 *
 *
 * @file          atomic_link_op.h
 * @brief         Atomic Link Operation  
 *
 * Author:        Atsushi Kasuya
 *
 */

#ifndef _ATOMIC_LINK_OP_H_
#define _ATOMIC_LINK_OP_H_

#ifdef __cplusplus 
extern "C" {
#endif

#ifdef NETRONOME_HOST

//typedef unsigned int            uint32_t;
//typedef unsigned long long int  uint64_t;

#define __lmem 

#define ENTL_DEBUG(fmt, args...) printf( fmt, ## args )

#else

//#define ENTL_DEBUG(fmt, args...) 

#endif  // NETRONOME_HOST
#include <stdint.h>

#define ALO_OPCODE(x)  ((x >>16)&0xffff)
#define ALO_DT(x)      ((x >> 36) & 0x1f)
#define ALO_SC(x)       ((x >> 42) & 0x1f)

// Opcode definitions
#define ALO_TYPE(x)       ((x>>12) & 0xf)

#define ALO_NOP             0x0000
#define ALO_ADD             0x0001
#define ALO_SUBS            0x0002
#define ALO_SUBD            0x0003
#define ALO_AND             0x0004
#define ALO_OR              0x0005
#define ALO_INC             0x0006
#define ALO_DEC             0x0007
#define ALO_INCS            0x0008
#define ALO_INCD            0x0009
#define ALO_DECS            0x000a
#define ALO_DECD            0x000b
#define ALO_RD              0x1000
#define ALO_WR              0x2000
#define ALO_SWAP            0x3000
#define ALO_BSET            0x4000
#define ALO_BRESET          0x5000
#define ALO_BTESTSET        0x6000
#define ALO_BTESTRESET      0x7000

// Condition Code Definitions
#define ALO_COND_MASK       0x000f
#define ALO_COND_ALLWAYS    0x0000 
#define ALO_COND_EQ         0x0001
#define ALO_COND_LT         0x0002
#define ALO_COND_LE         0x0003
#define ALO_COND_GT         0x0004
#define ALO_COND_GE         0x0005

#define ALO_COND_OP_MASK    0xf000

#define ALO_BIT_MASK        0x003f


typedef struct alo_regs {
  uint64_t reg[32] ;
  uint64_t sent_value ;  // keep the sent value of alo_initiate for retry
  uint64_t result_buffer ;
  uint64_t return_value ;  // keep the result value of alo_exec for retry
  uint32_t return_flag ;
  uint32_t flags ;
  uint32_t state ;        
  uint32_t sent_flag ;
} alo_regs_t ;


void alo_regs_init( __lmem volatile alo_regs_t *alr ) ;

// result of alo_exec flags
#define ALO_RESULT_ALO_SUCCESS  0x0001
#define ALO_RESULT_ALO_FAIL     0x0000

// Initiate Atomic Link Operation at source
uint32_t alo_initiate( __lmem volatile alo_regs_t *alr, uint16_t opcode, uint16_t sr, uint64_t *s_value ) ;

int alo_next( __lmem volatile alo_regs_t *alr, uint16_t status, uint64_t r_value ) ;

int alo_complete( __lmem volatile alo_regs_t *alr ) ;

// Execute Atomic Link Operation at destination 
uint32_t alo_exec( __lmem volatile alo_regs_t *alr, uint16_t opcode, uint16_t dt, uint64_t s_value, uint64_t *r_value  ) ;

int alo_state(__lmem volatile alo_regs_t *alr ) ;

int alo_update( __lmem volatile alo_regs_t *alr, uint16_t status ) ; 

#ifdef __cplusplus 
}
#endif

#endif /* _ENTL_STATE_MACHINE_H_ */

/* -*-  Mode:C; c-basic-offset:4; tab-width:4 -*- */
