/*
 * Copyright (C) 2018,  Earth Computing Inc.  All rights reserved.
 *
 *
 * @file          ec_link_tester.c
 * @brief         EC_Link Tester
 *
 * Author:        Atsushi Kasuya
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "ec_link_tester.h"

#define ENTL_DEBUG(fmt, args...) if(debug_flag) printf( fmt, ## args )
static debug_flag = 1 ;

static char *entt_str( unsigned int entt ) 
{
	switch( entt )
	{
		case 0: return "ENTL" ;
		case 1: return "ENTT_Ra" ;
		case 2: return "ENTT_Rb" ;
		default: return "Unknown" ;
	}

}

void dump_state( char *str, ec_link_reg_t *reg ) 
{
	ENTL_DEBUG( "%s EC_LINK Reg:\n", str ) ;
	ENTL_DEBUG( "  sr = %d\n", reg->sr ) ;
	ENTL_DEBUG( "  rr = %d\n", reg->rr ) ;
	ENTL_DEBUG( "  tt = %d\n", reg->tt ) ;
	ENTL_DEBUG( "  tf = %d\n", reg->tf ) ;
	ENTL_DEBUG( "  token = %d\n", reg->token ) ;
	ENTL_DEBUG( "  entt = %s\n", entt_str(reg->entt) ) ;
	ENTL_DEBUG( "  entt_bkoff = %d\n", reg->entt_bkoff ) ;
	ENTL_DEBUG( "\n" ) ;
}

int expect_state( char *str, ec_link_reg_t *reg, uint8_t sr, uint8_t rr, uint8_t tt, uint8_t tf, uint8_t token, uint16_t tc, uint8_t entt, uint8_t entt_bkoff ) 
{
	if( reg->sr != sr ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->sr %d != sr %d\n", reg->sr, sr ) ;
		return 0 ;		
	}
	if( reg->rr != rr ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->rr %d != rr %d\n", reg->rr, rr ) ;
		return 0 ;		
	}
	if( reg->tt != tt ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->tt %d != tt %d\n", reg->tt, tt ) ;
		return 0 ;		
	}
	if( reg->tf != tf ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->tf %d != tf %d\n", reg->tf, tf ) ;
		return 0 ;		
	}
	if( reg->token != token ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->token %d != token %d\n", reg->token, token ) ;
		return 0 ;		
	}
	if( reg->tc != tc ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->tc %d != tc %d\n", reg->tc, tc ) ;
		return 0 ;		
	}
	if( reg->entt != entt ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->entt %d != entt %d\n", reg->entt, entt ) ;
		return 0 ;		
	}
	if( reg->entt_bkoff != entt_bkoff ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->entt_bkoff %d != entt_bkoff %d\n", reg->entt_bkoff, entt_bkoff ) ;
		return 0 ;		
	}


	return 1 ;
}

int check_field( char *str, ec_link_field_t *fld, uint8_t valid, uint8_t s_or_r, uint8_t value, uint8_t tecktack, uint8_t token ) 
{
	if( fld->valid != valid ) {
		ENTL_DEBUG( "check_field %s : ", str ) ;
		ENTL_DEBUG( "fld->valid %d != valid %d\n", fld->valid, valid ) ;
		return 0 ;			
	}
	if( fld->s_or_r != s_or_r ) {
		ENTL_DEBUG( "check_field %s : ", str ) ;
		ENTL_DEBUG( "fld->s_or_r %d != s_or_r %d\n", fld->s_or_r, s_or_r ) ;
		return 0 ;			
	}
	if( fld->value != value ) {
		ENTL_DEBUG( "check_field %s : ", str ) ;
		ENTL_DEBUG( "fld->value %d != value %d\n", fld->value, value ) ;
		return 0 ;			
	}
	if( fld->tecktack != tecktack ) {
		ENTL_DEBUG( "check_field %s : ", str ) ;
		ENTL_DEBUG( "fld->tecktack %d != tecktack %d\n", fld->tecktack, tecktack ) ;
		return 0 ;			
	}
	if( fld->token != token ) {
		ENTL_DEBUG( "check_field %s : ", str ) ;
		ENTL_DEBUG( "fld->token %d != token %d\n", fld->token, token ) ;
		return 0 ;			
	}
	return 1 ;

}

int healthy_loop( ec_link_reg_t *reg ) 
{
	int ret, action ;
	ec_link_field_t fld_in ;
	ec_link_field_t fld_out ;

	//ec_link_init( reg ) ;

    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "", reg,  0,  0,  0,  0,     0,  0,    0,          0 ) ;
	if( !ret ) return ret ;

	ec_link_send( reg, &fld_out ) ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "", &fld_out, 1,     1,      1,       0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "", reg,  0,  1,  1,  0,     0,  0,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "", &fld_out, 1,     0,      1,       0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "", reg,  1,  1,  0,  1,     0,  0,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "", &fld_out, 1,     1,      0,       0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "", reg,  1,  0,  1,  1,     0,  0,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "", &fld_out, 1,     0,      0,       0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "", reg,  0,  0,  0,  1,     0,  0,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "", &fld_out, 1,     1,      1,       0,     0 ) ;
	if( !ret ) return ret ;


    return ret ;
}

int tt_hiccup( ec_link_reg_t *reg ) 
{
	int ret, action ;
	ec_link_field_t fld_in ;
	ec_link_field_t fld_out ;

	//ec_link_init( reg ) ;

    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "", reg,  0,  0,  0,  0,     0,  0,    0,          0 ) ;
	if( !ret ) return ret ;

	ec_link_send( reg, &fld_out ) ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "", &fld_out, 1,     1,      1,       0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "", reg,  0,  1,  1,  0,     0,  0,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "", &fld_out, 1,     0,      1,       0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "", reg,  0,  0,  1,  0,     0,  1,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "", &fld_out, 1,     0,      0,       0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "", reg,  0,  1,  1,  0,     0,  2,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "", &fld_out, 1,     0,      1,       0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "", reg,  0,  0,  1,  0,     0,  3,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "", &fld_out, 1,     0,      0,       0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "", reg,  1,  0,  0,  1,     0,  0,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "", &fld_out, 1,     1,      0,       0,     0 ) ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "", reg,  0,  0,  0,  1,     0,  1,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "", &fld_out, 1,     1,      1,       0,     0 ) ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "", reg,  1,  0,  0,  1,     0,  2,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "", &fld_out, 1,     1,      0,       0,     0 ) ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "", reg,  0,  0,  0,  1,     0,  3,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "", &fld_out, 1,     1,      1,       0,     0 ) ;

/*
	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "", reg,  1,  0,  1,  1,     0,  0,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "", &fld_out, 1,     0,      0,       0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "", reg,  0,  0,  0,  1,     0,  0,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "", &fld_out, 1,     1,      1,       0,     0 ) ;
	if( !ret ) return ret ;
*/

    return ret ;
}

int tc_resend( ec_link_reg_t *reg ) 
{
	int ret, action ;
	ec_link_field_t fld_in ;
	ec_link_field_t fld_out ;

	//ec_link_init( reg ) ;

    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "0", reg,  0,  0,  0,  0,     0,  0,    0,          0 ) ;
	if( !ret ) return ret ;

	ec_link_send( reg, &fld_out ) ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "0", &fld_out, 1,     1,      1,       0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "1", reg,  0,  1,  1,  0,     0,  0,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "1", &fld_out, 1,     0,      1,       0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "2", reg,  0,  0,  1,  0,     0,  1,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "2", &fld_out, 1,     0,      0,       0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "3", reg,  0,  1,  1,  0,     0,  2,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "3", &fld_out, 1,     0,      1,       0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "4", reg,  0,  0,  1,  0,     0,  3,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "4", &fld_out, 1,     0,      0,       0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "5", reg,  0,  1,  1,  0,     0,  4,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "5", &fld_out, 1,     0,      1,       0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != ENTL_ACTION_RECOVER ) {
		ENTL_DEBUG( "Action %x != expected %x\n", action, ENTL_ACTION_RECOVER ) ;
		return 0 ;
	}
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "6", reg,  0,  0,  1,  0,     0,  0,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "6", &fld_out, 1,     0,      0,       0,     0 ) ;
	if( !ret ) return ret ;
	ec_link_resend( reg, fld_in, &fld_out ) ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "6.1", &fld_out, 1,     1,      1,       0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "7", reg,  1,  0,  0,  1,     0,  0,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "7", &fld_out, 1,     1,      0,       0,     0 ) ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "8", reg,  0,  0,  0,  1,     0,  1,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "8", &fld_out, 1,     1,      1,       0,     0 ) ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "9", reg,  1,  0,  0,  1,     0,  2,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "9", &fld_out, 1,     1,      0,       0,     0 ) ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "10", reg,  0,  0,  0,  1,     0,  3,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "10", &fld_out, 1,     1,      1,       0,     0 ) ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "11", reg,  1,  0,  0,  1,     0,  4,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "11", &fld_out, 1,     1,      0,       0,     0 ) ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != ENTL_ACTION_RECOVER ) {
		ENTL_DEBUG( "Action %x != expected %x\n", action, ENTL_ACTION_RECOVER ) ;
		return 0 ;
	}
	//int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "12", reg,  0,  0,  0,  0,     0,  0,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "12", &fld_out, 1,     1,      1,       0,     0 ) ;
	ec_link_resend( reg, fld_in, &fld_out ) ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "12.1", &fld_out, 1,     0,      0,       0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state(  "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "13", reg,  0,  1,  1,  0,     0,  0,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "13", &fld_out, 1,     0,      1,       0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state(  "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "14", reg,  1,  1,  0,  1,     0,  0,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "14", &fld_out, 1,     1,      0,       0,     0 ) ;

    return ret ;
}

int entt_loop( ec_link_reg_t *reg ) 
{
	int ret, action ;
	ec_link_field_t fld_in ;
	ec_link_field_t fld_out ;

	//ec_link_init( reg ) ;
	ENTL_DEBUG( "entt_loop\n" ) ;

    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "0", reg,  0,  0,  0,  0,     0,  0,    0,          0 ) ;
	if( !ret ) return ret ;

	ec_link_send( reg, &fld_out ) ;
	//int check_field(         fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "0", &fld_out, 1,     1,      1,       0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "1", reg,  0,  1,  1,  0,     0,  0,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field(       fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "1", &fld_out, 1,     0,      1,       0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "2", reg,  1,  1,  0,  1,     0,  0,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field(       fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "2", &fld_out, 1,     1,      0,       0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.tecktack = 1 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_RECV ) {
		ENTL_DEBUG( "Action %x != expected %x\n", action, ENTL_ACTION_ENTT_RECV ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff ) 
	ret = expect_state( "3", reg,  1,  0,  1,  1,     0,  0, ENTT_Rb,          0 ) ;
	if( !ret ) return ret ;
	//int check_field(           fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "3", &fld_out,     1,     0,      0,       1,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 1 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_FNSH ) {
		ENTL_DEBUG( "Action %x != expected %x\n", action, ENTL_ACTION_ENTT_FNSH ) ;
		return 0 ;
	}
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff ) 
	ret = expect_state( "4", reg,  0,  0,  0,  1,    1,  0,       0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field(       fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "4", &fld_out, 1,     1,      1,       0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "5", reg,  0,  1,  1,  1,     1,  0,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field(      fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "5", &fld_out, 1,     0,      1,       0,     0 ) ;
	if( !ret ) return ret ;

	// ENTT reject Sequence

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_STRT ) {
		ENTL_DEBUG( "Action %x != expected %x\n", action, ENTL_ACTION_ENTT_STRT ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "6", reg,  0,  1,  0,  1,     1,  0, ENTT_Ra,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, tecktack, token )
	ret = check_field( "6", &fld_out, 1,     1,      1,       1,     0 ) ;
	if( !ret ) return ret ;

	// 
	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "7", reg,  0,  0,  1,  1,     0,  0,    ENTT_Ra,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, tecktack, token )
	ret = check_field( "7", &fld_out, 1,     0,      0,       0,     0 ) ;
	if( !ret ) return ret ;

	// ENTT reject
	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_DROP ) {
		ENTL_DEBUG( "Action %x != expected %x\n", action, ENTL_ACTION_ENTT_DROP ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "8", reg,  1,  0,  0,  1,     0,  0, 0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, tecktack, token )
	ret = check_field( "8", &fld_out, 1,     1,      0,       0,     0 ) ;
	if( !ret ) return ret ;


	// ENTT Accepted sequence

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 1 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "9", reg, 1,  1,  1,  1,     1,  0,    0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field(       fld, valid, s_or_r, value, tecktack, token )
	ret = check_field( "9", &fld_out, 1,     0,      1,       0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_STRT ) {
		ENTL_DEBUG( "Action %x != expected %x\n", action, ENTL_ACTION_ENTT_STRT ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "10", reg,  0,  1,  0,  1,     1,  0, ENTT_Ra,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, tecktack, token )
	ret = check_field( "10", &fld_out, 1,     1,      1,       1,     0 ) ;
	if( !ret ) return ret ;

	// 
	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.tecktack = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "11", reg,  0,  0,  1,  1,     0,  0,    ENTT_Ra,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, tecktack, token )
	ret = check_field( "11", &fld_out, 1,     0,      0,       0,     0 ) ;
	if( !ret ) return ret ;

	// ENTT Accept
	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.tecktack = 1 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_DONE ) {
		ENTL_DEBUG( "Action %x != expected %x\n", action, ENTL_ACTION_ENTT_DONE ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff ) 
	ret = expect_state( "12", reg,  1,  0,  0,  1,     0,  0,   0,          0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, tecktack, token )
	ret = check_field( "12", &fld_out, 1,     1,      0,       0,     1 ) ;
	if( !ret ) return ret ;



	ENTL_DEBUG( "ret = %d\n", ret ) ;

    return ret ;
}

