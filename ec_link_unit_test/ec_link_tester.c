/*---------------------------------------------------------------------------------------------
 *  Copyright © 2018-present Earth Computing Corporation. All rights reserved.
 *  Licensed under the MIT License. See LICENSE.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "ec_link_tester.h"

#define ENTL_DEBUG(fmt, args...) if(debug_flag) printf( fmt, ## args )
static debug_flag = 1 ;

static char *entt_str( unsigned int state ) 
{
	switch( state )
	{
		case 0: return "ENTL" ;
		case 1: return "ENTT_Ra" ;
		case 2: return "ENTT_Rb" ;
		case 3: return "ENTT_Rp" ;
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
	ENTL_DEBUG( "  state = %s\n", entt_str(reg->state) ) ;
	ENTL_DEBUG( "  entt_bkoff = %d\n", reg->entt_bkoff ) ;
	ENTL_DEBUG( "  token_bkoff = %d\n", reg->token_bkoff ) ;
	ENTL_DEBUG( "\n" ) ;
}

int expect_state( char *str, ec_link_reg_t *reg, uint8_t sr, uint8_t rr, uint8_t tt, uint8_t tf, uint8_t token, uint16_t tc, uint8_t state, uint8_t entt_bkoff, uint8_t token_bkoff ) 
{
	int ret = 1 ;
	if( reg->sr != sr ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->sr %d != sr %d\n", reg->sr, sr ) ;
		ret = 0 ;		
	}
	if( reg->rr != rr ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->rr %d != rr %d\n", reg->rr, rr ) ;
		ret = 0 ;		
	}
	if( reg->tt != tt ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->tt %d != tt %d\n", reg->tt, tt ) ;
		ret = 0 ;		
	}
	if( reg->tf != tf ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->tf %d != tf %d\n", reg->tf, tf ) ;
		ret = 0 ;		
	}
	if( reg->token != token ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->token %d != token %d\n", reg->token, token ) ;
		ret = 0 ;		
	}
	if( reg->tc != tc ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->tc %d != tc %d\n", reg->tc, tc ) ;
		ret = 0 ;		
	}
	if( reg->state != state ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->entt %d != entt %d\n", reg->state, state ) ;
		ret = 0 ;		
	}
	if( reg->entt_bkoff != entt_bkoff ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->entt_bkoff %d != entt_bkoff %d\n", reg->entt_bkoff, entt_bkoff ) ;
		ret = 0 ;		
	}
	if( reg->token_bkoff != token_bkoff ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->token_bkoff %d != token_bkoff %d\n", reg->token_bkoff, token_bkoff ) ;
		ret = 0 ;		
	}

	return ret ;
}

int check_field( char *str, ec_link_field_t *fld, uint8_t valid, uint8_t s_or_r, uint8_t value, uint8_t recover, uint8_t entt, uint8_t token ) 
{
	int ret = 1 ;
	if( fld->valid != valid ) {
		ENTL_DEBUG( "check_field %s : ", str ) ;
		ENTL_DEBUG( "fld->valid %d != valid %d\n", fld->valid, valid ) ;
		ret = 0 ;			
	}
	if( fld->s_or_r != s_or_r ) {
		ENTL_DEBUG( "check_field %s : ", str ) ;
		ENTL_DEBUG( "fld->s_or_r %d != s_or_r %d\n", fld->s_or_r, s_or_r ) ;
		ret = 0 ;			
	}
	if( fld->value != value ) {
		ENTL_DEBUG( "check_field %s : ", str ) ;
		ENTL_DEBUG( "fld->value %d != value %d\n", fld->value, value ) ;
		ret = 0 ;			
	}
	if( fld->recover != recover ) {
		ENTL_DEBUG( "check_field %s : ", str ) ;
		ENTL_DEBUG( "fld->recover %d != recover %d\n", fld->recover, recover ) ;
		ret = 0 ;			
	}
	if( fld->entt != entt ) {
		ENTL_DEBUG( "check_field %s : ", str ) ;
		ENTL_DEBUG( "fld->entt %d != entt %d\n", fld->entt, entt ) ;
		ret = 0 ;			
	}
	if( fld->token != token ) {
		ENTL_DEBUG( "check_field %s : ", str ) ;
		ENTL_DEBUG( "fld->token %d != token %d\n", fld->token, token ) ;
		ret = 0 ;			
	}
	return ret ;

}

int healthy_loop( ec_link_reg_t *reg ) 
{
	int ret, action ;
	ec_link_field_t fld_in ;
	ec_link_field_t fld_out ;

	//ec_link_init( reg ) ;

    //int expect_state( str, reg, sr, rr, tt, tf, token, tc, state, entt_bkoff, token_bkoff ) ; 
	ret = expect_state(  "", reg,  0,  0,  0,  0,     0,  0,     0,          0,           0 ) ;
	if( !ret ) return ret ;

	ec_link_send( reg, &fld_out ) ;
	//int check_field( str, fld, valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "", &fld_out, 1,      1,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( str, reg, sr, rr, tt, tf, token, tc, state, entt_bkoff, token_bkoff ) ; 
	ret = expect_state(  "", reg,  0,  1,  1,  0,     0,  0,     0,          0,           0 ) ;
	if( !ret ) return ret ;
	//int check_field( str, fld, valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "", &fld_out, 1,     0,      1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTL_LKUP ) return 0 ;
    //int expect_state( str, reg, sr, rr, tt, tf, token, tc, state, entt_bkoff, token_bkoff ) ; 
	ret = expect_state(  "", reg,  1,  1,  0,  1,     0,  0,     0,          0,           0 ) ;
	if( !ret ) return ret ;
	//int check_field( str, fld, valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "", &fld_out, 1,      1,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( str, reg, sr, rr, tt, tf, token, tc, state, entt_bkoff, token_bkoff ) ; 
	ret = expect_state( "",  reg,  1,  0,  1,  1,     0,  0,     0,          0,           0 ) ;
	if( !ret ) return ret ;
	//int check_field( str, fld, valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "", &fld_out, 1,      0,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( str, reg, sr, rr, tt, tf, token, tc, state, entt_bkoff, token_bkoff ) ; 
	ret = expect_state(  "", reg,  0,  0,  0,  1,     0,  0,     0,          0,           0 ) ;
	if( !ret ) return ret ;
	//int check_field( str, fld, valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "", &fld_out, 1,      1,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;


    return ret ;
}

int tt_hiccup( ec_link_reg_t *reg ) 
{
	int ret, action ;
	ec_link_field_t fld_in ;
	ec_link_field_t fld_out ;

	//ec_link_init( reg ) ;

    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, state, entt_bkoff, token_bkoff ) 
	ret = expect_state( "", reg,  0,  0,  0,  0,     0,  0,     0,          0,           0 ) ;
	if( !ret ) return ret ;

	ec_link_send( reg, &fld_out ) ;
	//int check_field( str, fld, valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "", &fld_out, 1,      1,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	ENTL_DEBUG( "1  action %x\n", action ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "", reg,  0,  1,  1,  0,     0,  0,    0,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( str, fld, valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "", &fld_out, 1,      0,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTL_LKUP ) return 0 ;
    //int expect_state( str, reg, sr, rr, tt, tf, token, tc, state, entt_bkoff, token_bkoff ) ; 
	ret = expect_state(  "", reg,  1,  1,  0,  1,     0,  0,     0,          0,           0 ) ;
	if( !ret ) return ret ;
	//int check_field( str, fld, valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "", &fld_out, 1,      1,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	ENTL_DEBUG( "2  action %x\n", action ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "", reg,  0,  1,  0,  1,     0,  1,    0,          0,           0 ) ;
	if( !ret ) return ret ;
	//int check_field( str, fld, valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "", &fld_out, 1,     1,      1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	ENTL_DEBUG( "3  action %x\n", action ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "", reg,  1,  1,  0,  1,     0,  2,    0,          0,           0 ) ;
	if( !ret ) return ret ;
	//int check_field( str, fld, valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "", &fld_out, 1,     1,      0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	ENTL_DEBUG( "4  action %x\n", action ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "",   reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "oo", reg,  0,  1,  0,  1,     0,  3,    0,          0,  0 ) ;
	if( !ret ) return ret ;
	//int check_field( str, fld, valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "", &fld_out, 1,      1,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	ENTL_DEBUG( "5  action %x\n", action ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg,   sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff )  
	ret = expect_state( "op", reg,  0,  1,  1,  1,     0,  3,    0,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( str, fld, valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "", &fld_out, 1,      0,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	ENTL_DEBUG( "6  action %x\n", action ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "", reg,  0,  1,  0,  1,     0,  0,    0,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( str, fld, valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "", &fld_out, 1,     1,      1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	ENTL_DEBUG( "7  action %x\n", action ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "", reg,  1,  1,  0,  1,     0,  1,    0,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( str, fld, valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "", &fld_out, 1,     1,      0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	ENTL_DEBUG( "8  action %x\n", action ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "", reg,  1,  1,  1,  1,     0,  1,    0,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( str, fld, valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "xx", &fld_out, 1,    0,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	ENTL_DEBUG( "9  action %x\n", action ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "", reg,  1,  1,  0,  1,     0,  0,    0,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "xx", &fld_out, 1,      1,     0,       0,    0,     0 ) ;

    return ret ;
}

int tc_resend( ec_link_reg_t *reg ) 
{
	int ret, action ;
	ec_link_field_t fld_in ;
	ec_link_field_t fld_out ;

	//ec_link_init( reg ) ;

    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "0", reg,  0,  0,  0,  0,     0,  0,    0,          0, 0 ) ;
	if( !ret ) return ret ;

	ec_link_send( reg, &fld_out ) ;
	//int check_field( fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "0", &fld_out, 1,     1,      1,        0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff )
	ret = expect_state( "1", reg,  0,  1,  1,  0,     0,  0,    0,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "1", &fld_out, 1,     0,      1,        0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTL_LKUP ) return 0 ;
    //int expect_state( str, reg, sr, rr, tt, tf, token, tc, state, entt_bkoff, token_bkoff ) ; 
	ret = expect_state(  "", reg,  1,  1,  0,  1,     0,  0,     0,          0,           0 ) ;
	if( !ret ) return ret ;
	//int check_field( str, fld, valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "", &fld_out, 1,      1,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	ENTL_DEBUG( "2  action %x\n", action ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "", reg,  0,  1,  0,  1,     0,  1,    0,          0,           0 ) ;
	if( !ret ) return ret ;
	//int check_field( str, fld, valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "", &fld_out, 1,     1,      1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	ENTL_DEBUG( "3  action %x\n", action ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "", reg,  1,  1,  0,  1,     0,  2,    0,          0,           0 ) ;
	if( !ret ) return ret ;
	//int check_field( str, fld, valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "", &fld_out, 1,     1,      0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	ENTL_DEBUG( "4  action %x\n", action ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "", reg,  0,  1,  0,  1,     0,  3,    0,          0,           0 ) ;
	if( !ret ) return ret ;
	//int check_field( str, fld, valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "", &fld_out, 1,     1,      1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "5", reg,  1,  1,  0,  1,     0,  4,    0,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "5", &fld_out, 1,     1,      0,        0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != (ENTL_ACTION_ENTL_LKDN | ENTL_ACTION_ENTL_RECOVER) ) {
		ENTL_DEBUG( "Action %x != expected %x\n", action, ENTL_ACTION_ENTL_LKDN | ENTL_ACTION_ENTL_RECOVER ) ;
		return 0 ;
	}
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "6", reg,  0,  1,  0,  0,     0,  0,    0,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "6", &fld_out, 1,     1,      1,        0,    0,     0 ) ;
	if( !ret ) return ret ;

	ret = ec_link_recover( reg, &fld_out ) ;
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "6", &fld_out, 1,     0,      1,        1,    0,     0 ) ;
	if( !ret ) return ret ;


	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	ENTL_DEBUG( "5  action %x\n", action ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff )
	ret = expect_state( "1", reg,  0,  0,  1,  0,     0,  0,    0,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "1", &fld_out, 1,     0,      0,        0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	ENTL_DEBUG( "7  action %x\n", action ) ;
	if( action != ENTL_ACTION_ENTL_LKUP ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "7", reg,  1,  0,  0,  1,     0,  0,    0,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "7", &fld_out, 1,     1,      0,        0,    0,     0 ) ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	ENTL_DEBUG( "8  action %x\n", action ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "8", reg,  0,  0,  0,  1,     0,  1,    0,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "8", &fld_out, 1,     1,      1,        0,    0,     0 ) ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	ENTL_DEBUG( "9  action %x\n", action ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "9", reg,  1,  0,  0,  1,     0,  2,    0,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "9", &fld_out, 1,     1,      0,        0,    0,     0 ) ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	ENTL_DEBUG( "10  action %x\n", action ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "10", reg,  0,  0,  0,  1,     0,  3,    0,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "10", &fld_out, 1,     1,      1,       0,    0,     0 ) ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	ENTL_DEBUG( "11  action %x\n", action ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "11", reg,  1,  0,  0,  1,     0,  4,    0,        0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "11", &fld_out, 1,     1,      0,       0,    0,     0 ) ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != (ENTL_ACTION_ENTL_RECOVER | ENTL_ACTION_ENTL_LKDN) ) {
		ENTL_DEBUG( "Action %x != expected %x\n", action, (ENTL_ACTION_ENTL_RECOVER | ENTL_ACTION_ENTL_LKDN) ) ;
		return 0 ;
	}
	//int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "12", reg,  0,  0,  0,  0,     0,  0,    0,        0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "12", &fld_out, 1,     1,      1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	ENTL_DEBUG( "13  action %x\n", action ) ;
	if( action != 0 ) return 0 ;
    //int expect_state(  "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "13", reg,  0,  1,  1,  0,     0,  0,    0,         0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "13", &fld_out, 1,     0,      1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	ENTL_DEBUG( "14  action %x\n", action ) ;
	if( action != ENTL_ACTION_ENTL_LKUP ) return 0 ;
    //int expect_state(  "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "14", reg,  1,  1,  0,  1,     0,  0,    0,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "14", &fld_out, 1,     1,      0,       0,    0,     0 ) ;

    return ret ;
}

int entt_loop( ec_link_reg_t *reg ) 
{
	int ret, action ;
	int i ;
	ec_link_field_t fld_in ;
	ec_link_field_t fld_out ;

	//ec_link_init( reg ) ;
	ENTL_DEBUG( "entt_loop\n" ) ;

    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "0", reg,  0,  0,  0,  0,     0,  0,    0,          0, 0 ) ;
	if( !ret ) return ret ;

	ec_link_send( reg, &fld_out ) ;
	//int check_field(         fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "0", &fld_out, 1,     1,      1,        0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "1", reg,  0,  1,  1,  0,     0,  0,    0,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field(       fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "1", &fld_out, 1,     0,      1,        0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTL_LKUP ) return 0 ;
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "2", reg,  1,  1,  0,  1,     0,  0,    0,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field(       fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "2", &fld_out, 1,     1,      0,        0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.entt = 1 ;
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_RECV ) {
		ENTL_DEBUG( "3 Action %x != expected %x\n", action, ENTL_ACTION_ENTT_RECV ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "3", reg,  1,  0,  1,  1,     0,  0, ENTT_Rb,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field(           fld, valid, s_or_r, value, entt, token )
	//int check_field( str,    fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "3", &fld_out,     1,      0,     0,       0,    1,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( !ret ) return ret ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "4", reg,  0,  0,  0,  1,    0,  0, ENTT_Rb,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field(       fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "4", &fld_out,  1,     1,      1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.entt = 2 ;
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_FNSH ) {
		ENTL_DEBUG( "5 Action %x != expected %x\n", action, ENTL_ACTION_ENTT_FNSH ) ;
		//return 0 ;
	}
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "5", reg,  0,  1,  1,  1,     0,  0,    0,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field(      fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "5", &fld_out,  1,      0,     1,       0,    2,     0 ) ;
	if( !ret ) return ret ;

	// ENTT reject Sequence

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 1 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_STRT ) {
		ENTL_DEBUG( "6 Action %x != expected %x\n", action, ENTL_ACTION_ENTT_STRT ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "6", reg,  0,  1,  0,  1,     1,  0, ENTT_Ra,          0, 1 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "6", &fld_out,  1,      1,     1,       0,    1,     0 ) ;
	if( !ret ) return ret ;

	// 
	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "7", reg,  0,  0,  1,  1,     1,  0,    ENTT_Ra,          0, 1 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "7", &fld_out,  1,      0,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	// ENTT reject
	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_DROP ) {
		ENTL_DEBUG( "8 Action %x != expected %x\n", action, ENTL_ACTION_ENTT_DROP ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "8", reg,  1,  0,  0,  1,     1,  0,    0,          0, 1 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "8", &fld_out,  1,      1,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;


	// ENTT Accepted sequence
	ENTL_DEBUG( "Pass here 0\n" );

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "9", reg, 1,  1,  1,  1,     1,  0,    0,          0, 1 ) ;
	if( !ret ) return ret ;
	//int check_field(       fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "9", &fld_out,  1,      0,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_STRT ) {
		ENTL_DEBUG( "10 Action %x != expected %x\n", action, ENTL_ACTION_ENTT_STRT ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "10", reg,  0,  1,  0,  1,     1,  0, ENTT_Ra,          0, 2 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "10", &fld_out, 1,      1,     1,       0,    1,     0 ) ;
	if( !ret ) return ret ;

	// 
	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.entt = 1 ;    // entt request should be rejected
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) 
	{
		ENTL_DEBUG( "11 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;		
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "11", reg,  0,  0,  1,  1,     1,  0,    ENTT_Ra,   0, 2 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "11", &fld_out, 1,      0,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;
	ENTL_DEBUG( "Pass here 1\n" );

	// ENTT Accept
	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 1 ;    // accept
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != 0 ) 
	{
		ENTL_DEBUG( "12 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;		
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "12", reg,  1,  0,  0,  1,     1,  0, ENTT_Rp,          0, 2 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "12", &fld_out, 1,      1,     0,       0,    2,     0 ) ;
	if( !ret ) return ret ;

	// 
	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) 
	{
		ENTL_DEBUG( "13 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;		
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "13", reg,  1,  1,  1,  1,     1,  0,    ENTT_Rp,   0, 2 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "13", &fld_out, 1,      0,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	// ENTT Complete
	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 2 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_DONE ) {
		ENTL_DEBUG( "14 Action %x != expected %x\n", action, ENTL_ACTION_ENTT_DONE ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "14", reg,  0,  1,  0,  1,     1,  0, ENTT_NIL,         0, 2 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "14", &fld_out, 1,      1,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	ENTL_DEBUG( "Pass here 2\n" );

	// Timeover to start ENTT
	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "13", reg,  0,  0,  1,  1,     0,  0,    0,        0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field(       fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "13", &fld_out, 1,      0,     0,       0,    0,     1 ) ;  // token is released 
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "14", reg,  1,  0,  0,  1,     0,  0,    0,         1, 0 ) ;
	if( !ret ) return ret ;
	//int check_field(       fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "14", &fld_out, 1,      1,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "15", reg,  1,  1,  1,  1,     0,  0,    0,        1, 0 ) ;
	if( !ret ) return ret ;
	//int check_field(       fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "15", &fld_out, 1,      0,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "16", reg,  0,  1,  0,  1,     0,  0,    0,         2, 0 ) ;
	if( !ret ) return ret ;
	//int check_field(       fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "16", &fld_out, 1,      1,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "17", reg,  0,  0,  1,  1,     0,  0,    0,        2, 0 ) ;
	if( !ret ) return ret ;
	//int check_field(       fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "17", &fld_out, 1,      0,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "18", reg,  1,  0,  0,  1,     0,  0,    0,         3, 0 ) ;
	if( !ret ) return ret ;
	//int check_field(       fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "18", &fld_out, 1,      1,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "19", reg,  1, 1,  1,  1,     0,  0,    0,        3, 0 ) ;
	if( !ret ) return ret ;
	//int check_field(       fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "19", &fld_out, 1,      0,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "20", reg,  0,  1,  0,  1,     0,  0,    0,         4, 0 ) ;
	if( !ret ) return ret ;
	//int check_field(       fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "20", &fld_out, 1,      1,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "21", reg,  0,  0,  1,  1,     0,  0,    0,        4, 0 ) ;
	if( !ret ) return ret ;
	//int check_field(       fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "21", &fld_out, 1,      0,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_STRT ) {
		ENTL_DEBUG( "22 Action %x != expected %x\n", action, ENTL_ACTION_ENTT_STRT ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "22", reg,  1,  0,  0,  1,     0,  0, ENTT_Ra,      0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field(       fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "22", &fld_out, 1,      1,     0,       0,    1,     0 ) ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "23", reg,  1,  1,  1,  1,     0,  0,    ENTT_Ra,   0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "23", &fld_out, 1,      0,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	// ENTT Accept
	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 1 ;
	fld_in.token = 1 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) 
	{
		ENTL_DEBUG( "24 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;		
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "24", reg,  0,  1,  0,  1,     1,  0, ENTT_Rp,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "24", &fld_out, 1,      1,     1,       0,    2,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "25", reg,  0,  0,  1,  1,     1,  0,    ENTT_Rp,   0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "25", &fld_out, 1,      0,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	// ENTT Accept
	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 2 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_DONE ) {
		ENTL_DEBUG( "26 Action %x != expected %x\n", action, ENTL_ACTION_ENTT_DONE ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "26", reg,  1,  0,  0,  1,     1,  0,   0,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "26", &fld_out, 1,      1,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	// Error recovery to cancell ENTT receive case
	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.entt = 1 ;
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_RECV ) {
		ENTL_DEBUG( "27 Action %x != expected %x\n", action, ENTL_ACTION_ENTT_RECV ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "27", reg,  1,  1,  1,  1,     0,  0, ENTT_Rb,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field(           fld, valid, s_or_r, value, entt, token )
	//int check_field( str,    fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "27", &fld_out,     1,      0,     1,       0,    1,     1 ) ;
	if( !ret ) return ret ;

	for( i = 0 ; i < 5 ; i++ ) 
	{
		fld_in.valid = 1 ;
		fld_in.s_or_r = 0 ;
		fld_in.value = i & 1 ;
		fld_in.entt = 0 ;
		fld_in.token = 0 ;
		fld_in.recover = 0 ;
		action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
		if( action != 0 ) 
		{
			ENTL_DEBUG( "28 Action %x != expected %x\n", action, 0 ) ;
			return 0 ;		
		}
	    //int expect_state( "",   reg,     sr, rr, tt, tf, token, tc,    entt, entt_bkoff, token_bkoff ) 
		ret = expect_state( "28", reg,  i & 1,  1,  0,  1,     0,  i, ENTT_Rb,          0, 0 ) ;
		if( !ret ) return ret ;
		//int check_field(       fld, valid, s_or_r, value, entt, token )
		//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
		ret = check_field( "28", &fld_out,  1,     1,      (i + 1) & 1,       0,    0,     0 ) ;
		if( !ret ) return ret ;		
	}

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != (ENTL_ACTION_ENTL_LKDN | ENTL_ACTION_ENTL_RECOVER) ) {
		ENTL_DEBUG( "Action %x != expected %x\n", action, ENTL_ACTION_ENTL_LKDN | ENTL_ACTION_ENTL_RECOVER ) ;
		return 0 ;
	}
    //int expect_state( "",   reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "29", reg,  1,  1,  0,  0,     0,  0, ENTT_Rb,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "29", &fld_out, 1,     1,      0,        0,    0,     0 ) ;
	if( !ret ) return ret ;

	// recover packet field generation
	action = ec_link_recover( reg, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_DROP ) {
		ENTL_DEBUG( "Action %x != expected %x\n", action, ENTL_ACTION_ENTT_DROP ) ;
		return 0 ;
	}
    //int expect_state( "",   reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "30", reg,  1,  1,  0,  0,     0,  0, ENTT_NIL,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "30", &fld_out, 1,     0,      1,        1,    2,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != 0 ) {
		ENTL_DEBUG( "31 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "31", reg,  1,  0,  1,  0,     0,  0, ENTT_NIL,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field(           fld, valid, s_or_r, value, entt, token )
	//int check_field( str,    fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "31", &fld_out,     1,      0,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTL_LKUP ) {
		ENTL_DEBUG( "32 Action %x != expected %x\n", action, ENTL_ACTION_ENTL_LKUP ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "32", reg,  0,  0,  0,  1,     0,  0,    0,         0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field(       fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "32", &fld_out, 1,      1,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != 0 ) {
		ENTL_DEBUG( "33 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "33", reg,  0,  1,  1,  1,     0,  0, ENTT_NIL,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field(           fld, valid, s_or_r, value, entt, token )
	//int check_field( str,    fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "33", &fld_out,     1,      0,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	//
	// Error recovery on ENTT Initiation, drop at Ra case
	//
	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 1 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_STRT ) {
		ENTL_DEBUG( "34 Action %x != expected %x\n", action, ENTL_ACTION_ENTT_STRT ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "34", reg,  1,  1,  0,  1,     1,  0, ENTT_Ra,          0, 1 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "34", &fld_out, 1,      1,     0,       0,    1,     0 ) ;
	if( !ret ) return ret ;

	// 
	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.entt = 1 ;    // entt request should be rejected
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) 
	{
		ENTL_DEBUG( "35 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;		
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "35", reg,  1,  0,  1,  1,     1,  0,    ENTT_Ra,   0, 1 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "35", &fld_out, 1,      0,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	// ENTT fail with recover
	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;    // accept
	fld_in.token = 0 ;
	fld_in.recover = 1 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_DROP ) 
	{
		ENTL_DEBUG( "36 Action %x != expected %x\n", action, ENTL_ACTION_ENTT_DROP ) ;
		return 0 ;		
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "36", reg,  1,  0,  0,  1,     1,  0, ENTT_NIL,          0, 1 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "36", &fld_out, 1,      1,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;    
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) 
	{
		ENTL_DEBUG( "37 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;		
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "37", reg,  1,  1,  1,  1,     1,  0,    ENTT_NIL,   0, 1 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "37", &fld_out, 1,      0,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	//
	// Error recovery on ENTT Initiation, drop at Rp case
	//
	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 1 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_STRT ) {
		ENTL_DEBUG( "38 Action %x != expected %x\n", action, ENTL_ACTION_ENTT_STRT ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "38", reg,  0,  1,  0,  1,     1,  0, ENTT_Ra,          0, 1 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "38", &fld_out, 1,      1,     1,       0,    1,     0 ) ;
	if( !ret ) return ret ;

	// 
	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.entt = 1 ;    // entt request should be rejected
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) 
	{
		ENTL_DEBUG( "39 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;		
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "39", reg,  0,  0,  1,  1,     1,  0,    ENTT_Ra,   0, 1 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "39", &fld_out, 1,      0,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	// ENTT Accept
	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 1 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) 
	{
		ENTL_DEBUG( "40 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;		
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "40", reg,  0,  0,  0,  1,     1,  0, ENTT_Rp,          0, 1 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "40", &fld_out, 1,      1,     1,       0,    2,     0 ) ;
	if( !ret ) return ret ;

	// 
	for( i = 0 ; i < 6 ; i++ ) {
		fld_in.valid = 1 ;
		fld_in.s_or_r = 1 ;
		fld_in.value = (i)&1 ;
		fld_in.entt = 0 ;
		fld_in.token = 0 ;
		fld_in.recover = 0 ;
		action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
		if( action != 0 ) 
		{
			ENTL_DEBUG( "41 Action %x != expected %x\n", action, 0 ) ;
			return 0 ;		
		}
	    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
		ret = expect_state( "41", reg,  0,  (i)&1,  1,  1,     1,  0,    ENTT_Rp,   0, 1 ) ;
		if( !ret ) return ret ;
		//int check_field( fld,       valid, s_or_r, value, entt, token )
		//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
		ret = check_field( "41", &fld_out, 1,      0,     (i)&1,       0,    0,     0 ) ;
		if( !ret ) return ret ;
	}

	// ENTT Complete with error
	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 2 ;
	fld_in.token = 0 ;
	fld_in.recover = 1 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_DROP ) {
		ENTL_DEBUG( "42 Action %x != expected %x\n", action, ENTL_ACTION_ENTT_DROP ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "42", reg,  1,  1,  0,  1,     1,  0, ENTT_NIL,         0, 1 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "42", &fld_out, 1,      1,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;    
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) 
	{
		ENTL_DEBUG( "43 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;		
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "43", reg,  1,  0,  1,  1,     1,  0,    ENTT_NIL,   0, 1 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "43", &fld_out, 1,      0,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

// working here
	//
	// Error recovery on ENTT Initiation, complete at Rp case
	//
	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_STRT ) {
		ENTL_DEBUG( "38 Action %x != expected %x\n", action, ENTL_ACTION_ENTT_STRT ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "38", reg,  0,  0,  0,  1,     1,  0, ENTT_Ra,          0, 2 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "38", &fld_out, 1,      1,     1,       0,    1,     0 ) ;
	if( !ret ) return ret ;

	// 
	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;    // entt request should be rejected
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) 
	{
		ENTL_DEBUG( "39 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;		
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "39", reg,  0,  0,  1,  1,     1,  0,    ENTT_Ra,   0, 2 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "39", &fld_out, 1,      0,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	// ENTT Accept
	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 1 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) 
	{
		ENTL_DEBUG( "40 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;		
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "40", reg,  0,  0,  0,  1,     1,  0, ENTT_Rp,          0, 2 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "40", &fld_out, 1,      1,     1,       0,    2,     0 ) ;
	if( !ret ) return ret ;

	// 
	for( i = 0 ; i < 6 ; i++ ) {
		fld_in.valid = 1 ;
		fld_in.s_or_r = 1 ;
		fld_in.value = (i)&1 ;
		fld_in.entt = 0 ;
		fld_in.token = 0 ;
		fld_in.recover = 0 ;
		action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
		if( action != 0 ) 
		{
			ENTL_DEBUG( "41 Action %x != expected %x\n", action, 0 ) ;
			return 0 ;		
		}
	    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
		ret = expect_state( "41", reg,  0,  (i)&1,  1,  1,     1,  0,    ENTT_Rp,   0, 2 ) ;
		if( !ret ) return ret ;
		//int check_field( fld,       valid, s_or_r, value, entt, token )
		//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
		ret = check_field( "41", &fld_out, 1,      0,     (i)&1,       0,    0,     0 ) ;
		if( !ret ) return ret ;
	}

	// ENTT Complete with error, no ENTA
	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	fld_in.recover = 1 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_DONE ) {
		ENTL_DEBUG( "42 Action %x != expected %x\n", action, ENTL_ACTION_ENTT_DONE ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "42", reg,  1,  1,  0,  1,     1,  0, ENTT_NIL,         0,  2) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "42", &fld_out, 1,      1,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;


	// token release 
	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "43", reg,  1,  0,  1,  1,     0,  0,    0,         0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "43", &fld_out, 1,      0,     0,       0,    0,     1 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != 0 ) {
		ENTL_DEBUG( "44 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "44", reg,  0,  0,  0,  1,     0,  0, 0,        0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "44", &fld_out, 1,      1,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	// Request reject on queue full
	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.entt = 1 ;
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 0, 1, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc,        entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "45", reg,  0,  1,  1,  1,    0,  0,    ENTT_NIL,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "45", &fld_out, 1,      0,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	// ENTT Accept
	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != 0 ) {
		ENTL_DEBUG( "46 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "46", reg,  1,  1,  0,  1,    0,  0,   0,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "46", &fld_out, 1,      1,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.entt = 1 ;
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_RECV ) {
		ENTL_DEBUG( "46 Action %x != expected %x\n", action, ENTL_ACTION_ENTT_RECV ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc,        entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "47", reg,  1,  0,  1,  1,     0,  0, ENTT_Rb,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "47", &fld_out, 1,      0,     0,       0,    1,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) {
		ENTL_DEBUG( "48 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "48", reg,  0,  0,  0,  1,     0,  0, ENTT_Rb,        0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "48", &fld_out, 1,      1,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;
	if( action != 0 ) return 0 ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.entt = 2 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_FNSH ) {
		ENTL_DEBUG( "49 Action %x != expected %x\n", action, ENTL_ACTION_ENTT_FNSH ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc,        entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "49", reg,  0,  0,  1,  1,     0,  0,    0,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "49", &fld_out, 1,      0,     0,       0,    2,     0 ) ;
	if( !ret ) return ret ;

    //dump_state( "Last ", reg ) ;

	ENTL_DEBUG( "ret = %d\n", ret ) ;

    return ret ;
}

int entt_token( ec_link_reg_t *reg ) 
{
	int ret, action ;
	int i ;
	ec_link_field_t fld_in ;
	ec_link_field_t fld_out ;

	//ec_link_init( reg ) ;
	ENTL_DEBUG( "entt_token\n" ) ;
	    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "0", reg,  0,  0,  0,  0,     0,  0,    0,          0, 0 ) ;
	if( !ret ) return ret ;

	ec_link_send( reg, &fld_out ) ;
	//int check_field(         fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "0", &fld_out, 1,     1,      1,        0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "1", reg,  0,  1,  1,  0,     0,  0,    0,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field(       fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "1", &fld_out, 1,     0,      1,        0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 1 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTL_LKUP ) return 0 ;
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "2", reg,  1,  1,  0,  1,     1,  0,    0,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field(       fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "2", &fld_out, 1,     1,      0,        0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "3", reg, 1,  0,  1,  1,     1,  0,    0,          0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field(       fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "3", &fld_out,  1,      0,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_STRT ) {
		ENTL_DEBUG( "4 Action %x != expected %x\n", action, ENTL_ACTION_ENTT_STRT ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "4", reg,  0,  0,  0,  1,     1,  0, ENTT_Ra,          0, 1 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "4", &fld_out, 1,      1,     1,       0,    1,     0 ) ;
	if( !ret ) return ret ;

	// 
	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.entt = 1 ;    // entt request should be rejected
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) 
	{
		ENTL_DEBUG( "5 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;		
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "5", reg,  0,  0,  1,  1,     1,  0,    ENTT_Ra,   0, 1 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "5", &fld_out, 1,      0,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;
	ENTL_DEBUG( "Pass here 1\n" );

	// ENTT Accept
	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 1 ;    // accept
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != 0 ) 
	{
		ENTL_DEBUG( "6 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;		
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "6", reg,  1,  0,  0,  1,     1,  0, ENTT_Rp,          0, 1 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "6", &fld_out, 1,      1,     0,       0,    2,     0 ) ;
	if( !ret ) return ret ;

	// 
	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) 
	{
		ENTL_DEBUG( "7 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;		
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "7", reg,  1,  1,  1,  1,     1,  0,    ENTT_Rp,   0, 1 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "7", &fld_out, 1,      0,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	// ENTT Complete
	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 2 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_DONE ) {
		ENTL_DEBUG( "8 Action %x != expected %x\n", action, ENTL_ACTION_ENTT_DONE ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "8", reg,  0,  1,  0,  1,     1,  0, ENTT_NIL,         0, 1 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "8", &fld_out, 1,      1,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "9", reg, 0,  0,  1,  1,     1,  0,    0,          0, 1 ) ;
	if( !ret ) return ret ;
	//int check_field(       fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "9", &fld_out,  1,      0,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_STRT ) {
		ENTL_DEBUG( "10 Action %x != expected %x\n", action, ENTL_ACTION_ENTT_STRT ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "10", reg,  1,  0,  0,  1,     1,  0, ENTT_Ra,          0, 2 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "10", &fld_out, 1,      1,     0,       0,    1,     0 ) ;
	if( !ret ) return ret ;

	// 
	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.entt = 1 ;    // entt request should be rejected
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) 
	{
		ENTL_DEBUG( "11 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;		
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "11", reg,  1,  1,  1,  1,     1,  0,    ENTT_Ra,   0, 2 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "11", &fld_out, 1,      0,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;
	ENTL_DEBUG( "Pass here 1\n" );

	// ENTT Accept
	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 1 ;    // accept
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != 0 ) 
	{
		ENTL_DEBUG( "12 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;		
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "12", reg,  0,  1,  0,  1,     1,  0, ENTT_Rp,          0, 2 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "12", &fld_out, 1,      1,     1,       0,    2,     0 ) ;
	if( !ret ) return ret ;

	// 
	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) 
	{
		ENTL_DEBUG( "13 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;		
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "13", reg,  0,  0,  1,  1,     1,  0,    ENTT_Rp,   0, 2 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "13", &fld_out, 1,      0,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	// ENTT Complete
	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 2 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_DONE ) {
		ENTL_DEBUG( "14 Action %x != expected %x\n", action, ENTL_ACTION_ENTT_DONE ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "14", reg,  1,  0,  0,  1,     1,  0, ENTT_NIL,         0, 2 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "14", &fld_out, 1,      1,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	fld_in.recover = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) return 0 ;
    //int expect_state( "", reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "15", reg, 1,  1,  1,  1,     1,  0,    0,          0,  2) ;
	if( !ret ) return ret ;
	//int check_field(       fld, valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "15", &fld_out,  1,      0,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_STRT ) {
		ENTL_DEBUG( "16 Action %x != expected %x\n", action, ENTL_ACTION_ENTT_STRT ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "16", reg,  0,  1,  0,  1,     1,  0, ENTT_Ra,          0, 3 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "16", &fld_out, 1,      1,     1,       0,    1,     0 ) ;
	if( !ret ) return ret ;

	// 
	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.entt = 1 ;    // entt request should be rejected
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) 
	{
		ENTL_DEBUG( "17 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;		
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "17", reg,  0,  0,  1,  1,     1,  0,    ENTT_Ra,   0, 3 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "17", &fld_out, 1,      0,     0,       0,    0,     0 ) ;
	if( !ret ) return ret ;
	ENTL_DEBUG( "Pass here 1\n" );

	// ENTT Accept
	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 1 ;
	fld_in.entt = 1 ;    // accept
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != 0 ) 
	{
		ENTL_DEBUG( "18 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;		
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "18", reg,  1,  0,  0,  1,     1,  0, ENTT_Rp,          0, 3 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "18", &fld_out, 1,      1,     0,       0,    2,     0 ) ;
	if( !ret ) return ret ;

	// 
	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 1 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) 
	{
		ENTL_DEBUG( "19 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;		
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "19", reg,  1,  1,  1,  1,     1,  0,    ENTT_Rp,   0, 3 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "19", &fld_out, 1,      0,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	// ENTT Complete
	fld_in.valid = 1 ;
	fld_in.s_or_r = 0 ;
	fld_in.value = 0 ;
	fld_in.entt = 2 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 0, 0, &fld_out ) ;
	if( action != ENTL_ACTION_ENTT_DONE ) {
		ENTL_DEBUG( "20 Action %x != expected %x\n", action, ENTL_ACTION_ENTT_DONE ) ;
		return 0 ;
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "20", reg,  0,  1,  0,  1,     1,  0, ENTT_NIL,         0, 3 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "20", &fld_out, 1,      1,     1,       0,    0,     0 ) ;
	if( !ret ) return ret ;

	fld_in.valid = 1 ;
	fld_in.s_or_r = 1 ;
	fld_in.value = 0 ;
	fld_in.entt = 0 ;
	fld_in.token = 0 ;
	action = ec_link_action( reg, fld_in, 1, 0, &fld_out ) ;
	if( action != 0 ) 
	{
		ENTL_DEBUG( "21 Action %x != expected %x\n", action, 0 ) ;
		return 0 ;		
	}
    //int expect_state( "",  reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff, token_bkoff ) 
	ret = expect_state( "21", reg,  0,  0,  1,  1,     0,  0,    ENTT_NIL,   0, 0 ) ;
	if( !ret ) return ret ;
	//int check_field( fld,       valid, s_or_r, value, entt, token )
	//int check_field( str, fld,   valid, s_or_r, value, recover, entt, token ) ;
	ret = check_field( "21", &fld_out, 1,      0,     0,       0,    0,     1 ) ;
	if( !ret ) return ret ;

    return ret ;

}



