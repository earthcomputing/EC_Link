/*
 * Copyright (C) 2017,2018,  Earth Computing Inc.  All rights reserved.
 *
 *
 * @file          alo_tester.h
 * @brief         Atomic Link Operation Tester
 *
 * Author:        Atsushi Kasuya
 *
 */

#ifndef _ALO_TESTER_H_
#define _ALO_TESTER_H_

#include "atomic_link_op.h"

#ifdef __cplusplus 
extern "C" {
#endif

void dump_regs( alo_regs_t *eng, char *name ) ;

void clear_regs( alo_regs_t *eng ) ;

void rand_regs( alo_regs_t *eng ) ;

void copy_regs( alo_regs_t *source, alo_regs_t *destination ) ;

int check_untouched( alo_regs_t *source, alo_regs_t *destination, uint16_t except ) ;

int check_cond( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt, uint16_t opcode ) ;

int alo_exec_ops( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt, uint16_t opcode ) ;

int alo_check_result( char *name, alo_regs_t *eng, uint16_t reg, uint64_t expect , uint32_t expect_flags ) ;

int get_bit( alo_regs_t *eng, uint16_t reg, uint16_t bit ) ;


int alo_nop_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt ) ;
int alo_add_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt ) ;
int alo_subs_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt ) ;
int alo_subd_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt ) ;
int alo_and_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt ) ;
int alo_or_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt ) ;
int alo_inc_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt ) ;
int alo_dec_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt ) ;
int alo_incs_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt ) ;
int alo_incd_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt ) ;
int alo_decs_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt ) ;
int alo_decd_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt ) ;


int alo_rd_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt, uint16_t cond ) ;
int alo_wr_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt, uint16_t cond ) ;
int alo_swap_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt, uint16_t cond ) ;

int alo_bset_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt, uint16_t bit ) ;
int alo_breset_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt, uint16_t bit ) ;

int alo_btestset_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt, uint16_t bit ) ;
int alo_btestreset_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt, uint16_t bit ) ;


#ifdef __cplusplus 
}
#endif

#endif /* _ALO_TESTER_H_ */

