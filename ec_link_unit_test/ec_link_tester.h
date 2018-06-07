/*
 * Copyright (C) 2018,  Earth Computing Inc.  All rights reserved.
 *
 *
 * @file          ec_link_tester.h
 * @brief         EC_Link Tester
 *
 * Author:        Atsushi Kasuya
 *
 */

#ifndef _EC_LINK_TESTER_H_
#define _EC_LINK_TESTER_H_

#include "ec_link.h"

#ifdef __cplusplus 
extern "C" {
#endif

void dump_state( char *str, ec_link_reg_t *reg ) ;

int expect_state( char *str, ec_link_reg_t *reg, uint8_t sr, uint8_t rr, uint8_t tt, uint8_t tf, uint8_t token, uint16_t tc, uint8_t state, uint8_t entt_bkoff, uint8_t token_bkoff ) ; 

int check_field( char *str, ec_link_field_t *fld, uint8_t valid, uint8_t s_or_r, uint8_t value, uint8_t recover, uint8_t entt, uint8_t token ) ;

int healthy_loop( ec_link_reg_t *reg ) ;

int tt_hiccup( ec_link_reg_t *reg ) ;

int tc_resend( ec_link_reg_t *reg ) ;

int entt_loop( ec_link_reg_t *reg ) ;

int entt_token( ec_link_reg_t *reg ) ;
#ifdef __cplusplus 
}
#endif

#endif /* _EC_LINK_TESTER_H_ */