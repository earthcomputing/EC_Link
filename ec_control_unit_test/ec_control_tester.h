/*
 * Copyright (C) 2018,  Earth Computing Inc.  All rights reserved.
 *
 *
 * @file          ec_control_tester.h
 * @brief         EC_Control Tester
 *
 * Author:        Atsushi Kasuya
 *
 */

#ifndef _EC_CONTROL_TESTER_H_
#define _EC_CONTROL_TESTER_H_

#include "ec_control.h"
#include "alo_tester.h"

#ifdef __cplusplus 
extern "C" {
#endif

void dump_state( char *str, entl_state_machine_t *mcn ) ;

int expect_link_state( char *str, entl_state_machine_t *mcn, ec_link_reg_t *lnk ) ; 

int ec_alo_check_result( char *name, entl_state_machine_t *mcn, uint16_t reg, uint64_t expect , uint32_t expect_flags ) ;

void gen_addr( uint64_t *addr, uint8_t valid, uint8_t s_or_r, uint8_t value, uint8_t tecktack, uint8_t token, uint32_t alo_command, uint8_t recover ) ;

int check_daddr( char *str, uint64_t daddr, uint8_t valid, uint8_t s_or_r, uint8_t value, uint8_t tecktack, uint8_t token, uint32_t alo_command, uint8_t recover ) ;

int healthy_loop( entl_state_machine_t *mcn0, entl_state_machine_t *mcn1 ) ;

int tt_hiccup( entl_state_machine_t *mcn0, entl_state_machine_t *mcn1 ) ;

int tc_resend( entl_state_machine_t *mcn0, entl_state_machine_t *mcn1 ) ;

int entt_ait_sequence( entl_state_machine_t *mcn0, entl_state_machine_t *mcn1 ) ;

int entt_alo_sequence( entl_state_machine_t *mcn0, entl_state_machine_t *mcn1 ) ;

int entt_alo_resend( entl_state_machine_t *mcn0, entl_state_machine_t *mcn1 ) ;

#ifdef __cplusplus 
}
#endif

#endif /* _EC_CONTROL_TESTER_H_ */