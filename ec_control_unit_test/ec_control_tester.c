/*
 * Copyright (C) 2018,  Earth Computing Inc.  All rights reserved.
 *
 *
 * @file          ec_control_tester.c
 * @brief         EC Link Controller Tester
 *
 * Author:        Atsushi Kasuya
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "cyc_random.h"
#include "ec_control.h"
#include "ec_control_tester.h"
#define ENTL_DEBUG(fmt, args...) if(debug_flag) printf( fmt, ## args )

static int debug_flag = 1 ;

static char *entt_str( unsigned int entt ) 
{
	switch( entt )
	{
		case 0: return "ENTL" ;
		case 1: return "ENTT_Ra" ;
		case 2: return "ENTT_Rb" ;
		case 3: return "ENTT_Rp" ;
		default: return "Unknown" ;
	}
}

void dump_state( char *str, entl_state_machine_t *mcn ) 
{
    int i ;
    if( str ) ENTL_DEBUG( "Engine %s :\n", str) ;

	ENTL_DEBUG( "EC_LINK Reg:\n" ) ;
	ENTL_DEBUG( "  sr = %d\n", mcn->reg.sr ) ;
	ENTL_DEBUG( "  rr = %d\n", mcn->reg.rr ) ;
	ENTL_DEBUG( "  tt = %d\n", mcn->reg.tt ) ;
	ENTL_DEBUG( "  tf = %d\n", mcn->reg.tf ) ;
	ENTL_DEBUG( "  token = %d\n", mcn->reg.token ) ;
	ENTL_DEBUG( "  state = %d %s\n", mcn->reg.state, entt_str(mcn->reg.state) ) ;
	ENTL_DEBUG( "  entt_bkoff = %d\n", mcn->reg.entt_bkoff ) ;
	ENTL_DEBUG( "  token_bkoff = %d\n", mcn->reg.token_bkoff ) ;
	
	ENTL_DEBUG( "ALO_Reg\n" ) ;    
    for( i = 0 ; i < 32 ; i++ ) {
        ENTL_DEBUG( "  reg[%d] : %lx\n", i, mcn->ao.reg[i] ) ;
    }
    ENTL_DEBUG( "  result_buffer: %lx flags: %x state: %x \n", mcn->ao.result_buffer, mcn->ao.flags, mcn->ao.state) ;
    ENTL_DEBUG( "  return_value: %lx return_flag: %x state: %x \n", mcn->ao.return_value, mcn->ao.return_flag ) ;
	ENTL_DEBUG( "\n" ) ;    
	ENTL_DEBUG( "last_alo:%x last_d_addr:%lx \n",
		mcn->last_alo_command, mcn->last_d_addr 
	) ;    
	
    ENTL_DEBUG( "  recover_count: %lx recovered_count: %lx entt_count: %x aop_count: %x \n", mcn->recover_count, mcn->recovered_count, mcn->entt_count, mcn->aop_count ) ;

}

int expect_link_state( char *str, entl_state_machine_t *mcn, ec_link_reg_t *reg ) 
{
	int ret = 1 ;
	if( reg->sr != mcn->reg.sr ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->sr %d != sr %d\n", mcn->reg.sr, reg->sr ) ;
		ret = 0 ;		
	}
	if( reg->rr != mcn->reg.rr ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->rr %d != rr %d\n", mcn->reg.rr, reg->rr ) ;
		ret = 0 ;		
	}
	if( reg->tt != mcn->reg.tt ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->tt %d != tt %d\n", mcn->reg.tt, reg->tt ) ;
		ret = 0 ;		
	}
	if( reg->tf != mcn->reg.tf ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->tf %d != tf %d\n", mcn->reg.tf, reg->tf ) ;
		ret = 0 ;		
	}
	if( reg->token != mcn->reg.token ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->token %d != token %d\n", mcn->reg.token, reg->token ) ;
		ret = 0 ;		
	}
	if( reg->tc != mcn->reg.tc ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->tc %d != tc %d\n", mcn->reg.tc, reg->tc ) ;
		ret = 0 ;		
	}
	if( reg->state != mcn->reg.state ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->state %d != state %d\n", mcn->reg.state, reg->state ) ;
		ret = 0 ;		
	}
	if( reg->entt_bkoff != mcn->reg.entt_bkoff ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->entt_bkoff %d != entt_bkoff %d\n", mcn->reg.entt_bkoff, reg->entt_bkoff ) ;
		ret = 0 ;		
	}
	if( reg->token_bkoff != mcn->reg.token_bkoff ) {
		ENTL_DEBUG( "expect_state %s : ", str ) ;
		ENTL_DEBUG( "reg->token_bkoff %d != token_bkoff %d\n", mcn->reg.token_bkoff, reg->token_bkoff ) ;
		ret = 0 ;		
	}


	return ret ;
}

int ec_alo_check_result( char *name, entl_state_machine_t *mcn, uint16_t reg, uint64_t expect , uint32_t expect_flags ) 
{
    if( mcn->ao.reg[reg] != expect ) {
        ENTL_DEBUG( "alo_check_result %s reg %d value:%lx != expect %lx \n", name, reg, mcn->ao.reg[reg], expect ) ;
        return 0 ;
    }
    if( mcn->ao.flags != expect_flags ) {
        ENTL_DEBUG( "alo_check_result %s flags:%x != expect %x \n", name, mcn->ao.flags, expect_flags ) ;
        return 0 ;
    }
    return 1 ;
}

void gen_addr( uint64_t *addr, uint8_t valid, uint8_t s_or_r, uint8_t value, uint8_t entt, uint8_t token, uint32_t alo_command, uint8_t recover )
{
	ec_link_field_t field ;
	field._raw = 0 ;
	field.valid = valid ;
	field.s_or_r = s_or_r ;
	field.value = value ;
	field.entt = entt ;
	field.token = token ;
	field.recover = recover ;
	*addr = field._raw ;
    *addr |= SET_ECLP_ALO(alo_command) ;
    if( ALO_COMMAND_FW(alo_command) ) *addr |= ECLP_FW_MASK ;	
}

void dump_addr( char *name, uint64_t addr )
{
	ec_link_field_t field ;
	field._raw = addr ;

	ENTL_DEBUG( 
		" %s field valid %x s_or_r %x value %x entt %x token %x \n", 
		name, field.valid, field.s_or_r, field.value, field.entt, field.token 
	) ;

}

void dump_field( char *name, ec_link_field_t field )
{
	ENTL_DEBUG( 
		" %s field valid %x s_or_r %x value %x entt %x token %x, recover %x \n", 
		name, field.valid, field.s_or_r, field.value, field.entt, field.token, field.recover
	) ;	
}

int check_daddr( char *str, uint64_t daddr, uint8_t valid, uint8_t s_or_r, uint8_t value, uint8_t tecktack, uint8_t token, uint32_t alo_command, uint8_t recover ) 
{

	uint64_t addr ;
	ec_link_field_t field ;
	gen_addr( &addr, valid, s_or_r, value, tecktack, token, alo_command, recover ) ;
	field._raw = daddr ;
    if( daddr != addr ) {
        ENTL_DEBUG( "check_daddr %s daddr:%lx != expect %lx \n", str, daddr, addr ) ;
        dump_addr( str, daddr ) ;
        dump_addr( "expect", addr ) ;

        return 0 ;    	
    }
    return 1 ;
}

void set_state( ec_link_reg_t *reg, uint8_t sr, uint8_t rr, uint8_t tt, uint8_t tf, uint8_t token, uint16_t tc, uint8_t state, uint8_t entt_bkoff, uint8_t token_bkoff )
{
	reg->_raw = 0 ;
	reg->sr = sr ;
	reg->rr = rr ;
	reg->tt = tt ;
	reg->tf = tf ;
	reg->token = token ;
	reg->tc = tc ;
	reg->state = state ;
	reg->entt_bkoff = entt_bkoff ;
	reg->token_bkoff = token_bkoff ;
} 


int healthy_loop( entl_state_machine_t *mcn0, entl_state_machine_t *mcn1 ) 
{
	uint64_t addr, daddr0, daddr1 ;
	uint64_t data0, data1 ;
	int retval, ret ;
    ec_link_reg_t reg ;
	// initalize and first packet daddr


	entl_state_init( mcn0, &daddr0 ) ;
    //    check_daddr(     str,     daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval = check_daddr( "h_l 1", daddr0,     1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 1", mcn0, &reg ) ;

	entl_state_init( mcn1, &daddr1 ) ;
    //    check_daddr(     str,      daddr, valid, s_or_r, value, tecktack, token,  opcode )
	retval &= check_daddr( "h_l 2", daddr1,     1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
    retval &= expect_link_state( "h_l link 2", mcn1, &reg ) ;
       // entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) 

	//dump_state( "mcn0 0", mcn0 ) ;
	//dump_state( "mcn1 0", mcn1 ) ;

    // First packet received
	ret = entl_received( mcn0, daddr1,       0,         0,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 3", addr,   1,     0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 3", mcn0, &reg ) ;
	daddr1 = addr ;

	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 4", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
    retval &= expect_link_state( "h_l link 4", mcn1, &reg ) ;
	daddr0 = addr ;

    // Second packet received
	ret = entl_received( mcn0, daddr0,       0,         0,           0,            0, &addr, &data0 ) ;
	if( (ret & ENTL_ACTION_SIG_LUP ) != ENTL_ACTION_SIG_LUP ){
		ENTL_DEBUG( "entl_received ret = %x  expect %x\n", ret & ENTL_ACTION_SIG_LUP, ENTL_ACTION_SIG_LUP ) ;
		return 0 ;		
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 5", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 5", mcn0, &reg ) ;
	daddr0 = addr ;

	ret = entl_received( mcn1, daddr1,       0,         0,           0,            0, &addr, &data1 ) ;
	if( (ret & ENTL_ACTION_SIG_LUP ) != ENTL_ACTION_SIG_LUP ){
		ENTL_DEBUG( "entl_received ret = %x  expect %x\n", ret & ENTL_ACTION_SIG_LUP, ENTL_ACTION_SIG_LUP ) ;
		return 0 ;		
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 6", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
    retval &= expect_link_state( "h_l link 6", mcn1, &reg ) ;
	daddr1 = addr ;

    // Third packet received
	ret = entl_received( mcn0, daddr1,       0,         0,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 7", addr,   1,     0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 7", mcn0, &reg ) ;
	daddr1 = addr ;

	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 8", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
    retval &= expect_link_state( "h_l link 8", mcn1, &reg ) ;
	daddr0 = addr ;

    // Forth packet received
	ret = entl_received( mcn0, daddr0,       0,         0,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 9", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 9", mcn0, &reg ) ;
	daddr0 = addr ;

	ret = entl_received( mcn1, daddr1,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 10", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
    retval &= expect_link_state( "h_l link 10", mcn1, &reg ) ;
	daddr1 = addr ;

	//dump_state( "mcn0", mcn0 ) ;
	//dump_state( "mcn1", mcn1 ) ;

    return retval ;
}


int tt_hiccup( entl_state_machine_t *mcn0, entl_state_machine_t *mcn1 )
{
	uint64_t addr, daddr0, daddr1 ;
	uint64_t data0, data1 ;
	int retval, ret ;
    ec_link_reg_t reg ;

	entl_state_init( mcn0, &daddr0 ) ;
    //    check_daddr(     str,     daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval = check_daddr( "h_l 1", daddr0,     1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 1", mcn0, &reg ) ;

	entl_state_init( mcn1, &daddr1 ) ;
    //    check_daddr(     str,      daddr, valid, s_or_r, value, tecktack, token,  opcode )
	retval &= check_daddr( "h_l 2", daddr1,     1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
    retval &= expect_link_state( "h_l link 2", mcn1, &reg ) ;
       // entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) 

    // First packet received
	ret = entl_received( mcn0, daddr1,       0,         0,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 3", addr,   1,     0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 3", mcn0, &reg ) ;
	daddr1 = addr ;

	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 4", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
    retval &= expect_link_state( "h_l link 4", mcn1, &reg ) ;
	daddr0 = addr ;

    // Second packet received
	ret = entl_received( mcn0, daddr0,       0,         0,           0,            0, &addr, &data0 ) ;
	if( (ret & ENTL_ACTION_SIG_LUP ) != ENTL_ACTION_SIG_LUP ){
		ENTL_DEBUG( "entl_received ret = %x  expect %x\n", ret & ENTL_ACTION_SIG_LUP, ENTL_ACTION_SIG_LUP ) ;
		return 0 ;		
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 5", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 5", mcn0, &reg ) ;
	daddr0 = addr ;

	// Missing This Packet
	//ret = entl_received( mcn1, daddr1,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	//retval &= check_daddr( "h_l 6", addr,  1,      1,     0,        0,     0, ALO_NOP ) ;
    //retval &= expect_link_state( "h_l link 6", mcn1, &reg ) ;
	//daddr1 = addr ;

    // Third packet received
    // Missing this again
	//ret = entl_received( mcn0, daddr1,       0,         0,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	//retval &= check_daddr( "h_l 7", addr,   1,     0,     0,        0,     0, ALO_NOP ) ;
    //retval &= expect_link_state( "h_l link 7", mcn0, &reg ) ;
	//daddr1 = addr ;

	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 6", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  1,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 6", mcn1, &reg ) ;
	daddr0 = addr ;

	// Forth packet on mcn0
	ret = entl_received( mcn0, daddr0,       0,         0,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 7", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  0,  1,     0,  1,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 7", mcn0, &reg ) ;
	daddr0 = addr ;

	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 8", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 8", mcn1, &reg ) ;
	daddr0 = addr ;

	// Fifth packet on mcn0
	ret = entl_received( mcn0, daddr0,       0,         0,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 9", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  2,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 9", mcn0, &reg ) ;
	daddr0 = addr ;

	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 10", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  1,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 10", mcn1, &reg ) ;
	daddr0 = addr ;

	// Missing Packet resend
	ret = entl_received( mcn1, daddr1,       0,         0,           0,            0, &addr, &data1 ) ;
	if( (ret & ENTL_ACTION_SIG_LUP ) != ENTL_ACTION_SIG_LUP ){
		ENTL_DEBUG( "entl_received ret = %x  expect %x\n", ret & ENTL_ACTION_SIG_LUP, ENTL_ACTION_SIG_LUP ) ;
		return 0 ;		
	}
    //    check_daddr(     str,   daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 11", addr,    1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 11", mcn1, &reg ) ;
	daddr1 = addr ;

    // Missing returns
	ret = entl_received( mcn0, daddr1,       0,         0,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 12", addr,   1,     0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  2,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 12", mcn0, &reg ) ;
	daddr1 = addr ;	

	ret = entl_received( mcn0, daddr0,       0,         0,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 13", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 13", mcn0, &reg ) ;
	daddr0 = addr ;

	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 14", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 14", mcn1, &reg ) ;
	daddr0 = addr ;


	return retval ;
}

int tc_resend( entl_state_machine_t *mcn0, entl_state_machine_t *mcn1 ) {
	uint64_t addr, daddr0, daddr1 ;
	uint64_t data0, data1 ;
	int retval, ret ;
    ec_link_reg_t reg ;

	entl_state_init( mcn0, &daddr0 ) ;
    //    check_daddr(     str,     daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval = check_daddr( "h_l 1", daddr0,     1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 1", mcn0, &reg ) ;

	entl_state_init( mcn1, &daddr1 ) ;
    //    check_daddr(     str,      daddr, valid, s_or_r, value, tecktack, token,  opcode )
	retval &= check_daddr( "h_l 2", daddr1,     1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
    retval &= expect_link_state( "h_l link 2", mcn1, &reg ) ;
       // entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) 

    // First packet received
	ret = entl_received( mcn0, daddr1,       0,         0,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 3", addr,   1,     0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 3", mcn0, &reg ) ;
	daddr1 = addr ;

	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 4", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
    retval &= expect_link_state( "h_l link 4", mcn1, &reg ) ;
	daddr0 = addr ;

    // Second packet received
	ret = entl_received( mcn0, daddr0,       0,         0,           0,            0, &addr, &data0 ) ;
	if( (ret & ENTL_ACTION_SIG_LUP ) != ENTL_ACTION_SIG_LUP ){
		ENTL_DEBUG( "entl_received ret = %x  expect %x\n", ret & ENTL_ACTION_SIG_LUP, ENTL_ACTION_SIG_LUP ) ;
		return 0 ;		
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 5", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 5", mcn0, &reg ) ;
	daddr0 = addr ;

	// Missing This Packet
	//ret = entl_received( mcn1, daddr1,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	//retval &= check_daddr( "h_l 6", addr,  1,      1,     0,        0,     0, ALO_NOP ) ;
    //retval &= expect_link_state( "h_l link 6", mcn1, &reg ) ;
	//daddr1 = addr ;

    // Third packet received
    // Missing this again
	//ret = entl_received( mcn0, daddr1,       0,         0,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	//retval &= check_daddr( "h_l 7", addr,   1,     0,     0,        0,     0, ALO_NOP ) ;
    //retval &= expect_link_state( "h_l link 7", mcn0, &reg ) ;
	//daddr1 = addr ;

	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 6", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  1,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 6", mcn1, &reg ) ;
	daddr0 = addr ;

	// Forth packet on mcn0
	ret = entl_received( mcn0, daddr0,       0,         0,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 7", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  0,  1,     0,  1,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 7", mcn0, &reg ) ;
	daddr0 = addr ;

	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 8", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 8", mcn1, &reg ) ;
	daddr0 = addr ;

	// Fifth packet on mcn0
	ret = entl_received( mcn0, daddr0,       0,         0,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 9", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  2,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 9", mcn0, &reg ) ;
	daddr0 = addr ;

	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 10", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  1,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 10", mcn1, &reg ) ;
	daddr0 = addr ;

	// Sixth packet on mcn0
	ret = entl_received( mcn0, daddr0,       0,         0,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 11", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  0,  1,     0,  3,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 11", mcn0, &reg ) ;
	daddr0 = addr ;

	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 12", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 12", mcn1, &reg ) ;
	daddr0 = addr ;


	// Sixth packet on mcn0
	ret = entl_received( mcn0, daddr0,       0,         0,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 13", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  4,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 13", mcn0, &reg ) ;
	daddr0 = addr ;

	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 14", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  1,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 14", mcn1, &reg ) ;
	daddr0 = addr ;


	ret = entl_received( mcn0, daddr0,       0,         0,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 15", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  0,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 15", mcn0, &reg ) ;
	daddr0 = addr ;
	if( !(ret & ENTL_ACTION_RECOVER) ) {
		ENTL_DEBUG( "13 entl_received ret = %x\n", ret ) ;
		return 0 ;
	}
	if( !(ret & ENTL_ACTION_SIG_LDN) ) {
		ENTL_DEBUG( "13 entl_received ret = %x\n", ret ) ;
		return 0 ;
	}
		ENTL_DEBUG( "13 entl_received ret = %x\n", ret ) ;
	daddr0 = addr ;

	ret = entl_recover( mcn0, &addr ) ;
    //      check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 16", addr,  1,      0,     1,        0,     0, ALO_NOP, 1 ) ;
	daddr1 = addr ;

	// Missing Packet resend
	ret = entl_received( mcn1, daddr1,       0,         0,           0,            0, &addr, &data1 ) ;
	if( (ret & ENTL_ACTION_SIG_LUP ) != ENTL_ACTION_SIG_LUP ){
		ENTL_DEBUG( "16 entl_received ret = %x  expect %x\n", ret & ENTL_ACTION_SIG_LUP, ENTL_ACTION_SIG_LUP ) ;
		return 0 ;		
	}
		ENTL_DEBUG( "16 entl_received ret = %x  expect %x\n", ret, ENTL_ACTION_SIG_LUP ) ;

    //    check_daddr(     str,   daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 17", addr,    1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 17", mcn1, &reg ) ;
	daddr1 = addr ;

    // Missing returns
	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 18", addr,   1,     0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 18", mcn1, &reg ) ;
	daddr0 = addr ;	

	ret = entl_received( mcn0, daddr1,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 19", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  1,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 19", mcn0, &reg ) ;
	daddr1 = addr ;

	ret = entl_received( mcn1, daddr1,       0,         0,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 20", addr,   1,     1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 20", mcn1, &reg ) ;
	daddr1 = addr ;	

	ret = entl_received( mcn0, daddr0,       0,         0,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 21", addr,   1,     1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 21", mcn0, &reg ) ;
	daddr0 = addr ;	

	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 22", addr,   1,     0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 22", mcn1, &reg ) ;
	daddr0 = addr ;	

	return retval ;

}

int entt_ait_sequence( entl_state_machine_t *mcn0, entl_state_machine_t *mcn1 ) 
{
	uint64_t addr, daddr0, daddr1 ;
	uint64_t data0, data1 ;
	int retval, ret ;
    ec_link_reg_t reg ;

	entl_state_init( mcn0, &daddr0 ) ;
    //    check_daddr(     str,     daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval = check_daddr( "h_l 1", daddr0,     1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 1", mcn0, &reg ) ;

	entl_state_init( mcn1, &daddr1 ) ;
    //    check_daddr(     str,      daddr, valid, s_or_r, value, tecktack, token,  opcode )
	retval &= check_daddr( "h_l 2", daddr1,     1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
    retval &= expect_link_state( "h_l link 2", mcn1, &reg ) ;
       // entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) 

	//dump_state( "mcn0 0", mcn0 ) ;
	//dump_state( "mcn1 0", mcn1 ) ;

    // First packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,       0,         1,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 3", addr,   1,     0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 3", mcn0, &reg ) ;
	daddr1 = addr ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 4", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
    retval &= expect_link_state( "h_l link 4", mcn1, &reg ) ;
	daddr0 = addr ;

    // Second packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,       0,         1,           0,            0, &addr, &data0 ) ;
	if( (ret & ENTL_ACTION_SIG_LUP ) != ENTL_ACTION_SIG_LUP ){
		ENTL_DEBUG( "entl_received ret = %x  expect %x\n", ret & ENTL_ACTION_SIG_LUP, ENTL_ACTION_SIG_LUP ) ;
		return 0 ;		
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 5", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0,    0,          1,          0 ) ;
    retval &= expect_link_state( "h_l link 5", mcn0, &reg ) ;
	daddr0 = addr ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,       0,         0,           0,            0, &addr, &data1 ) ;
	if( (ret & ENTL_ACTION_SIG_LUP ) != ENTL_ACTION_SIG_LUP ){
		ENTL_DEBUG( "entl_received ret = %x  expect %x\n", ret & ENTL_ACTION_SIG_LUP, ENTL_ACTION_SIG_LUP ) ;
		return 0 ;		
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 6", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
    set_state(   &reg,  1,  1,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 6", mcn1, &reg ) ;
	daddr1 = addr ;

    // Third packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,       0,         1,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 7", addr,   1,     0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0,    0,          1,          0 ) ;
    retval &= expect_link_state( "h_l link 7", mcn0, &reg ) ;
	daddr1 = addr ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 8", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 8", mcn1, &reg ) ;
	daddr0 = addr ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,       0,         1,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 9", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    0,          2,          0 ) ;
    retval &= expect_link_state( "h_l link 9", mcn0, &reg ) ;
	daddr0 = addr ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 10", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 10", mcn1, &reg ) ;
	daddr1 = addr ;

    // First packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,       0,         1,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 11", addr,   1,     0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          2,          0 ) ;
    retval &= expect_link_state( "h_l link 11", mcn0, &reg ) ;
	daddr1 = addr ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 12", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 12", mcn1, &reg ) ;
	daddr0 = addr ;

    // Second packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,       0,         1,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 13", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0,    0,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 13", mcn0, &reg ) ;
	daddr0 = addr ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 14", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 14", mcn1, &reg ) ;
	daddr1 = addr ;

    // Third packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,       0,         1,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 15", addr,   1,     0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0,    0,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 15", mcn0, &reg ) ;
	daddr1 = addr ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 16", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 16", mcn1, &reg ) ;
	daddr0 = addr ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,       0,         1,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 17", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    0,          4,          0 ) ;
    retval &= expect_link_state( "h_l link 17", mcn0, &reg ) ;
	daddr0 = addr ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 18", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 18", mcn1, &reg ) ;
	daddr1 = addr ;

    // First packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,       0,         1,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 19", addr,   1,     0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          4,          0 ) ;
    retval &= expect_link_state( "h_l link 19", mcn0, &reg ) ;
	daddr1 = addr ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 20", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 20", mcn1, &reg ) ;
	daddr0 = addr ;

    // Second packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,       0,         1,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 21", addr,  1,      1,     0,        1,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff, token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0, ENTT_Ra,          0,           0 ) ;
    retval &= expect_link_state( "h_l link 21", mcn0, &reg ) ;
	daddr0 = addr ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 22", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff, token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0,       0,          0,           0 ) ;
    retval &= expect_link_state( "h_l link 22", mcn1, &reg ) ;
	daddr1 = addr ;

    // Third packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,       0,         1,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 23", addr,   1,     0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0, ENTT_Ra,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 23", mcn0, &reg ) ;
	daddr1 = addr ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 24", addr,  1,      0,     0,        1,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0, ENTT_Rb,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 24", mcn1, &reg ) ;
	daddr0 = addr ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,       0,         1,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 25", addr,  1,      1,     1,        2,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0, 	ENTT_Rp,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 25", mcn0, &reg ) ;
	daddr0 = addr ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 26", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0, ENTT_Rb,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 26", mcn1, &reg ) ;
	daddr1 = addr ;

    // Continuous ENTT sequence with token holding

    // First packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,       0,         1,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 27", addr,   1,     0,     1,        0,    0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0, ENTT_Rp,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 27", mcn0, &reg ) ;
	daddr1 = addr ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data1 ) ;
	if( (ret & (ENTL_ACTION_SIG_AIT | ENTL_ACTION_PROC_AIT) ) != (ENTL_ACTION_SIG_AIT | ENTL_ACTION_PROC_AIT) ) {
		ENTL_DEBUG( "entl_received ret = %x  expect %x\n", (ret & (ENTL_ACTION_SIG_AIT | ENTL_ACTION_PROC_AIT) ), (ENTL_ACTION_SIG_AIT | ENTL_ACTION_PROC_AIT) ) ;
		return 0 ;
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 28", addr,  1,      0,     1,        2,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 28", mcn1, &reg ) ;
	daddr0 = addr ;

    // Second packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,       0,         1,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 29", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     1,  0, ENTT_NIL,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 29", mcn0, &reg ) ;
	daddr0 = addr ;
	if( (ret & (ENTL_ACTION_SIG_AIT | ENTL_ACTION_DROP_AIT) ) != (ENTL_ACTION_SIG_AIT | ENTL_ACTION_DROP_AIT) ) {
		ENTL_DEBUG( "entl_received 29 ret = %x  expect %x\n", (ret ), (ENTL_ACTION_SIG_AIT | ENTL_ACTION_DROP_AIT) ) ;
		//return 0 ;
	}
		//ENTL_DEBUG( "entl_received 29 ret = %x  expect %x\n", (ret ), (ENTL_ACTION_SIG_AIT | ENTL_ACTION_DROP_AIT) ) ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 30", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 30", mcn1, &reg ) ;
	daddr1 = addr ;

    // Third packet received
    //  Token passing to mcn1
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,       0,         0,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 31", addr,   1,     0,     0,        0,     1, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0, ENTT_NIL,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 31", mcn0, &reg ) ;
	daddr1 = addr ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 32", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0, ENTT_NIL,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 32", mcn1, &reg ) ;
	daddr0 = addr ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,       0,         1,           0,            0, &addr, &data0 ) ;
	if( (ret & (ENTL_ACTION_SEND) ) != (ENTL_ACTION_SEND) ) {
		ENTL_DEBUG( "entl_received 33 ret = %x  expect %x\n", (ret ), (ENTL_ACTION_SEND) ) ;
		//return 0 ;
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 33", addr,  1,      1,     1,       0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,       entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    ENTT_NIL,          1,         0 ) ;
    retval &= expect_link_state( "h_l link 33", mcn0, &reg ) ;
	daddr0 = addr ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,       0,         1,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 34", addr,  1,      1,     1,        1,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     1,  0, ENTT_Ra,          0,          1 ) ;
    retval &= expect_link_state( "h_l link 34", mcn1, &reg ) ;
	daddr1 = addr ;


    // First packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,       0,         1,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 35", addr,   1,     0,     1,       1,    0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0, ENTT_Rb,          1,          0 ) ;
    retval &= expect_link_state( "h_l link 35", mcn0, &reg ) ;
	daddr1 = addr ;

	// Reject ENTT due to queue full
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,       0,         1,           0,            0, &addr, &data1 ) ;
	if( (ret & (ENTL_ACTION_SEND) ) != (ENTL_ACTION_SEND) ) {
		ENTL_DEBUG( "entl_received 36 ret = %x  expect %x\n", (ret ), (ENTL_ACTION_SEND) ) ;
		return 0 ;
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 36", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     1,  0, ENTT_Ra,          0,          1 ) ;
    retval &= expect_link_state( "h_l link 36", mcn1, &reg ) ;
	daddr0 = addr ;

	//dump_state( "mcn0 36", mcn0 ) ;

    // Second packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,       0,         1,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 37", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0, ENTT_Rb,          1,          0 ) ;
    retval &= expect_link_state( "h_l link 37", mcn0, &reg ) ;
	daddr0 = addr ;

	//dump_state( "mcn0 37", mcn0 ) ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,       0,         1,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 38", addr,  1,      1,     0,        2,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     1,  0, ENTT_Rp,          0,          1 ) ;
    retval &= expect_link_state( "h_l link 38", mcn1, &reg ) ;
	daddr1 = addr ;

    // Third packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,       0,         1,           0,            0, &addr, &data0 ) ;
	if( (ret & (ENTL_ACTION_SIG_AIT | ENTL_ACTION_PROC_AIT) ) != (ENTL_ACTION_SIG_AIT | ENTL_ACTION_PROC_AIT) ) {
		ENTL_DEBUG( "entl_received 39 ret = %x  expect %x\n", (ret & (ENTL_ACTION_SIG_AIT | ENTL_ACTION_PROC_AIT) ), (ENTL_ACTION_SIG_AIT | ENTL_ACTION_PROC_AIT) ) ;
		//return 0 ;
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 39", addr,   1,     0,     0,        2,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0,    0,          1,          0 ) ;
    retval &= expect_link_state( "h_l link 39", mcn0, &reg ) ;
	daddr1 = addr ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,       0,         1,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 40", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     1,  0, ENTT_Rp,          0,          1 ) ;
    retval &= expect_link_state( "h_l link 40", mcn1, &reg ) ;
	daddr0 = addr ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,       0,         1,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 41", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    0,          2,          0 ) ;
    retval &= expect_link_state( "h_l link 41", mcn0, &reg ) ;
	daddr0 = addr ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,       0,         1,           0,            0, &addr, &data1 ) ;
	if( (ret & (ENTL_ACTION_SIG_AIT | ENTL_ACTION_DROP_AIT) ) != (ENTL_ACTION_SIG_AIT | ENTL_ACTION_DROP_AIT) ) {
		ENTL_DEBUG( "entl_received ret = %x  expect %x\n", (ret & (ENTL_ACTION_SIG_AIT | ENTL_ACTION_DROP_AIT) ), (ENTL_ACTION_SIG_AIT | ENTL_ACTION_DROP_AIT) ) ;
		//return 0 ;
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 42", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     1,  0, ENTT_NIL,          0,          1 ) ;
    retval &= expect_link_state( "h_l link 42", mcn1, &reg ) ;
	daddr1 = addr ;

    // Third packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,       0,         1,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 43", addr,   1,     0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0, ENTT_NIL,          2,          0 ) ;
    retval &= expect_link_state( "h_l link 43", mcn0, &reg ) ;
	daddr1 = addr ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,       0,         1,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 44", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     1,  0, ENTT_NIL,          0,          1 ) ;
    retval &= expect_link_state( "h_l link 44", mcn1, &reg ) ;
	daddr0 = addr ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,       0,         1,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 45", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0, ENTT_NIL,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 45", mcn0, &reg ) ;
	daddr0 = addr ;

	// Recover case

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,       0,         1,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 46", addr,  1,      1,     0,        1,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     1,  0, ENTT_Ra,          0,          2 ) ;
    retval &= expect_link_state( "h_l link 46", mcn1, &reg ) ;
	daddr1 = addr ;

    // Third packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,       0,         1,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 47", addr,   1,     0,     0,        1,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0, ENTT_Rb,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 47", mcn0, &reg ) ;
	daddr1 = addr ;



    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,       0,         1,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 48", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     1,  0,  ENTT_Ra,          0,          2 ) ;
    retval &= expect_link_state( "h_l link 48", mcn1, &reg ) ;
	daddr0 = addr ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,       0,         1,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 49", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0, ENTT_Rb,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 49", mcn0, &reg ) ;
	daddr0 = addr ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,       0,         1,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 50", addr,  1,      1,     1,        2,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     1,  0, ENTT_Rp,          0,          2 ) ;
    retval &= expect_link_state( "h_l link 50", mcn1, &reg ) ;
	daddr1 = addr ;

    // Assume we lost the message above, receiver mcn0 stays in Rb

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,       0,         1,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 51", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     1,  0,  ENTT_Rp,          0,          2 ) ;
    retval &= expect_link_state( "h_l link 51", mcn1, &reg ) ;
	daddr0 = addr ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,       0,         1,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 52", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  0,  1,     0,  1, ENTT_Rb,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 52", mcn0, &reg ) ;
	daddr0 = addr ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,       0,         1,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 53", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  1,  1,     1,  0,  ENTT_Rp,          0,          2 ) ;
    retval &= expect_link_state( "h_l link 53", mcn1, &reg ) ;
	daddr0 = addr ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,       0,         1,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 54", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  2, ENTT_Rb,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 54", mcn0, &reg ) ;
	daddr0 = addr ;


    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,       0,         1,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 55", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     1,  0,  ENTT_Rp,          0,          2 ) ;
    retval &= expect_link_state( "h_l link 55", mcn1, &reg ) ;
	daddr0 = addr ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,       0,         1,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 56", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  0,  1,     0,  3, ENTT_Rb,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 56", mcn0, &reg ) ;
	daddr0 = addr ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,       0,         1,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 57", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  1,  1,     1,  0,  ENTT_Rp,          0,          2 ) ;
    retval &= expect_link_state( "h_l link 57", mcn1, &reg ) ;
	daddr0 = addr ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,       0,         1,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 58", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  4, ENTT_Rb,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 58", mcn0, &reg ) ;
	daddr0 = addr ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,       0,         1,           0,            0, &addr, &data1 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 59", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     1,  0,  ENTT_Rp,          0,          2 ) ;
    retval &= expect_link_state( "h_l link 59", mcn1, &reg ) ;
	daddr0 = addr ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,       0,         1,           0,            0, &addr, &data0 ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 60", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  0,  0,     0,  0,  ENTT_Rb,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 60", mcn0, &reg ) ;
	daddr0 = addr ;

	if( !(ret & ENTL_ACTION_RECOVER) ) {
		ENTL_DEBUG( "55 entl_received ret = %x\n", ret ) ;
		//return 0 ;
	}
	if( !(ret & ENTL_ACTION_SIG_LDN) ) {
		ENTL_DEBUG( "55 entl_received ret = %x\n", ret ) ;
		//return 0 ;
	}

	ret = entl_recover( mcn0, &addr ) ;
	if( (ret & ENTL_ACTION_CLER_AIT ) != ENTL_ACTION_CLER_AIT ){
		ENTL_DEBUG( "61 entl_received ret = %x  expect %x\n", ret , ENTL_ACTION_CLER_AIT ) ;
		//return 0 ;		
	}
    //      check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 61", addr,  1,      0,     0,        2,     0, ALO_NOP, 1 ) ;
	daddr1 = addr ;
    set_state(   &reg,  1,  0,  0,  0,     0,  0,  ENTT_NIL,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 61", mcn0, &reg ) ;

	// Missing Packet resend
	ret = entl_received( mcn1, daddr1,       0,         0,           0,            0, &addr, &data1 ) ;
		ENTL_DEBUG( "entl_received 62 ret = %x  expect %x\n", ret, ENTL_ACTION_CLER_AIT ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  0,  1,     1,  0,  ENTT_NIL,          0,          2 ) ;
    retval &= expect_link_state( "h_l link 59", mcn1, &reg ) ;



	//dump_state( "mcn0", mcn0 ) ;
	//dump_state( "mcn1", mcn1 ) ;

    return retval ;	
}

static alo_regs_t src_copy ;
static alo_regs_t dst_copy ;

int entt_alo_sequence( entl_state_machine_t *mcn0, entl_state_machine_t *mcn1 ) 
{
	uint64_t addr, daddr0, daddr1 ;
	uint64_t data, data0, data1, wr_data ;
	int retval, ret ;
    ec_link_reg_t reg ;

	entl_state_init( mcn0, &daddr0 ) ;
    //    check_daddr(     str,     daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval = check_daddr( "h_l 1", daddr0,     1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 1", mcn0, &reg ) ;

	entl_state_init( mcn1, &daddr1 ) ;
    //    check_daddr(     str,      daddr, valid, s_or_r, value, tecktack, token,  opcode )
	retval &= check_daddr( "h_l 2", daddr1,     1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
    retval &= expect_link_state( "h_l link 2", mcn1, &reg ) ;
       // entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) 

	//dump_state( "mcn0 0", mcn0 ) ;
	//dump_state( "mcn1 0", mcn1 ) ;

    rand_regs( &mcn0->ao ) ;
    rand_regs( &mcn1->ao ) ;

    copy_regs(&mcn0->ao, &src_copy) ;
    copy_regs(&mcn1->ao, &dst_copy) ;
    wr_data = mcn0->ao.reg[0] ;

    // First packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,       0,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 3", addr,   1,     0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 3", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 4", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
    retval &= expect_link_state( "h_l link 4", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Second packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         1,           0,            0, &addr, &data ) ;
	if( (ret & ENTL_ACTION_SIG_LUP ) != ENTL_ACTION_SIG_LUP ){
		ENTL_DEBUG( "entl_received 4 ret = %x  expect %x\n", ret & ENTL_ACTION_SIG_LUP, ENTL_ACTION_SIG_LUP ) ;
		return 0 ;		
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 5", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0,    0,          1,          0 ) ;
    retval &= expect_link_state( "h_l link 5", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
	if( (ret & ENTL_ACTION_SIG_LUP ) != ENTL_ACTION_SIG_LUP ){
		ENTL_DEBUG( "entl_received 5 ret = %x  expect %x\n", ret & ENTL_ACTION_SIG_LUP, ENTL_ACTION_SIG_LUP ) ;
		return 0 ;		
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 6", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
    set_state(   &reg,  1,  1,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 6", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // Third packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 7", addr,   1,     0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0,    0,          1,          0 ) ;
    retval &= expect_link_state( "h_l link 7", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 8", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 8", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 9", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    0,          2,          0 ) ;
    retval &= expect_link_state( "h_l link 9", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 10", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 10", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // First packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 11", addr,   1,     0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          2,          0 ) ;
    retval &= expect_link_state( "h_l link 11", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 12", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 12", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Second packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 13", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0,    0,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 13", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 14", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 14", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // Third packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 15", addr,   1,     0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0,    0,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 15", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 16", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 16", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 17", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    0,          4,          0 ) ;
    retval &= expect_link_state( "h_l link 17", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 18", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 18", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // First packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 19", addr,   1,     0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          4,          0 ) ;
    retval &= expect_link_state( "h_l link 19", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 20", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 20", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Second packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 21", addr,  1,      1,     0,        1,     0, ALO_WR, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff, token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0, ENTT_Ra,          0,           0 ) ;
    retval &= expect_link_state( "h_l link 21", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 22", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff, token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0,       0,          0,           0 ) ;
    retval &= expect_link_state( "h_l link 22", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // Third packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 23", addr,   1,     0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0, ENTT_Ra,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 23", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 24", addr,  1,      0,     0,        1,     0, 0x0001, 0 ) ; // set T flag
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0, ENTT_Rb,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 24", mcn1, &reg ) ;
	daddr0 = addr ;


    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 25", addr,  1,      1,     1,        2,     0, 0x0001, 0 ) ; // set T flag
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0, 	ENTT_Rp,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 25", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 26", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0, ENTT_Rb,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 26", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // Continuous ENTT sequence with token holding

    // First packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 27", addr,   1,     0,     1,        0,    0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0, ENTT_Rp,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 27", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         0,           0,            0, &addr, &data ) ;
	if( (ret & ( ENTL_ACTION_SIG_ALO ) ) != (ENTL_ACTION_SIG_ALO) ) {
		ENTL_DEBUG( "entl_received 27 ret = %x  expect %x\n", (ret  ), (ENTL_ACTION_SIG_ALO) ) ;
		//return 0 ;
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 28", addr,  1,      0,     1,        2,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 28", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Second packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         1,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 29", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     1,  0, ENTT_NIL,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 29", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

	if( (ret & (ENTL_ACTION_SIG_ALO ) ) != (ENTL_ACTION_SIG_ALO) ) {
		ENTL_DEBUG( "entl_received 29 ret = %x  expect %x\n", (ret ), (ENTL_ACTION_SIG_ALO) ) ;
		//return 0 ;
	}

    retval &= check_untouched( &mcn0->ao, &src_copy, 100 ) ;  
    retval &= check_untouched( &mcn1->ao, &dst_copy, 0 ) ;
    if( mcn1->ao.reg[0] != wr_data ) {
		ENTL_DEBUG( "mc1->ao.reg[0] = %lx  expect %lx\n", mcn1->ao.reg[0], wr_data ) ;
		retval = 0 ;	
    }

    rand_regs( &mcn0->ao ) ;
    rand_regs( &mcn1->ao ) ;

    copy_regs(&mcn0->ao, &src_copy) ;
    copy_regs(&mcn1->ao, &dst_copy) ;
    wr_data = mcn1->ao.reg[0] ;

		//ENTL_DEBUG( "entl_received 29 ret = %x  expect %x\n", (ret ), (ENTL_ACTION_SIG_AIT | ENTL_ACTION_DROP_AIT) ) ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 30", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 30", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // Third packet received
    //  Token passing to mcn1
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 31", addr,   1,     0,     0,        0,     1, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0, ENTT_NIL,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 31", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 32", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0, ENTT_NIL,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 32", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         0,      ALO_WR,            0, &addr, &data ) ;
	if( (ret & (ENTL_ACTION_SEND) ) != (ENTL_ACTION_SEND) ) {
		ENTL_DEBUG( "entl_received 33 ret = %x  expect %x\n", (ret ), (ENTL_ACTION_SEND) ) ;
		//return 0 ;
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 33", addr,  1,      1,     1,       0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,       entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    ENTT_NIL,          1,         0 ) ;
    retval &= expect_link_state( "h_l link 33", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 34", addr,  1,      1,     1,        1,     0, ALO_WR, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     1,  0, ENTT_Ra,          0,          1 ) ;
    retval &= expect_link_state( "h_l link 34", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // First packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 35", addr,   1,     0,     1,       1,     0,  0x0001, 0 ) ; // set T flag
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0, ENTT_Rb,          1,          0 ) ;
    retval &= expect_link_state( "h_l link 35", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

	// Reject ENTT due to queue full
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         1,           0,            0, &addr, &data ) ;
	if( (ret & (ENTL_ACTION_SEND) ) != (ENTL_ACTION_SEND) ) {
		ENTL_DEBUG( "entl_received 36 ret = %x  expect %x\n", (ret ), (ENTL_ACTION_SEND) ) ;
		return 0 ;
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 36", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     1,  0, ENTT_Ra,          0,          1 ) ;
    retval &= expect_link_state( "h_l link 36", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

	//dump_state( "mcn0 36", mcn0 ) ;

    // Second packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 37", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0, ENTT_Rb,          1,          0 ) ;
    retval &= expect_link_state( "h_l link 37", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

	//dump_state( "mcn0 37", mcn0 ) ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 38", addr,  1,      1,     0,        2,     0, 0x0001, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     1,  0, ENTT_Rp,          0,          1 ) ;
    retval &= expect_link_state( "h_l link 38", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // Third packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         1,           0,            0, &addr, &data ) ;
	if( (ret & (ENTL_ACTION_SIG_ALO) ) != (ENTL_ACTION_SIG_ALO) ) {
		ENTL_DEBUG( "entl_received 39 ret = %x  expect %x\n", (ret ), (ENTL_ACTION_SIG_ALO) ) ;
		//return 0 ;
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 39", addr,   1,     0,     0,        2,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0,    0,          1,          0 ) ;
    retval &= expect_link_state( "h_l link 39", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 40", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     1,  0, ENTT_Rp,          0,          1 ) ;
    retval &= expect_link_state( "h_l link 40", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 41", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    0,          2,          0 ) ;
    retval &= expect_link_state( "h_l link 41", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         1,           0,            0, &addr, &data ) ;
	if( (ret & (ENTL_ACTION_SIG_ALO) ) != (ENTL_ACTION_SIG_ALO) ) {
		ENTL_DEBUG( "entl_received 41 ret = %x  expect %x\n", (ret ), (ENTL_ACTION_SIG_ALO) ) ;
		//return 0 ;
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 42", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     1,  0, ENTT_NIL,          0,          1 ) ;
    retval &= expect_link_state( "h_l link 42", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    retval &= check_untouched( &mcn0->ao, &src_copy, 0 ) ;  
    retval &= check_untouched( &mcn1->ao, &dst_copy, 100 ) ;
    if( mcn1->ao.reg[0] != wr_data ) {
		ENTL_DEBUG( "mc1->ao.reg[0] = %lx  expect %lx\n", mcn1->ao.reg[0], wr_data ) ;
		retval = 0 ;	
    }

    rand_regs( &mcn0->ao ) ;
    rand_regs( &mcn1->ao ) ;

    copy_regs(&mcn0->ao, &src_copy) ;
    copy_regs(&mcn1->ao, &dst_copy) ;
    wr_data = mcn1->ao.reg[0] ;


    // Third packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 43", addr,   1,     0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0, ENTT_NIL,          2,          0 ) ;
    retval &= expect_link_state( "h_l link 43", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 44", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     1,  0, ENTT_NIL,          0,          1 ) ;
    retval &= expect_link_state( "h_l link 44", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 45", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0, ENTT_NIL,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 45", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

	// Recover case

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 46", addr,  1,      1,     0,        1,     0, ALO_WR, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     1,  0, ENTT_Ra,          0,          2 ) ;
    retval &= expect_link_state( "h_l link 46", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // Third packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 47", addr,   1,     0,     0,        1,     0, 0x0001, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0, ENTT_Rb,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 47", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;



    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 48", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     1,  0,  ENTT_Ra,          0,          2 ) ;
    retval &= expect_link_state( "h_l link 48", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 49", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0, ENTT_Rb,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 49", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 50", addr,  1,      1,     1,        2,     0, 0x0001, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     1,  0, ENTT_Rp,          0,          2 ) ;
    retval &= expect_link_state( "h_l link 50", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // Assume we lost the message above, receiver mcn0 stays in Rb

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 51", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     1,  0,  ENTT_Rp,          0,          2 ) ;
    retval &= expect_link_state( "h_l link 51", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 52", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  0,  1,     0,  1, ENTT_Rb,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 52", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 53", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  1,  1,     1,  0,  ENTT_Rp,          0,          2 ) ;
    retval &= expect_link_state( "h_l link 53", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 54", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  2, ENTT_Rb,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 54", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;


    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 55", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     1,  0,  ENTT_Rp,          0,          2 ) ;
    retval &= expect_link_state( "h_l link 55", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 56", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  0,  1,     0,  3, ENTT_Rb,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 56", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 57", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  1,  1,     1,  0,  ENTT_Rp,          0,          2 ) ;
    retval &= expect_link_state( "h_l link 57", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 58", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  4, ENTT_Rb,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 58", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 59", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     1,  0,  ENTT_Rp,          0,          2 ) ;
    retval &= expect_link_state( "h_l link 59", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 60", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  0,  0,     0,  0,  ENTT_Rb,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 60", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

	if( !(ret & ENTL_ACTION_RECOVER) ) {
		ENTL_DEBUG( "55 entl_received ret = %x\n", ret ) ;
		//return 0 ;
	}
	if( !(ret & ENTL_ACTION_SIG_LDN) ) {
		ENTL_DEBUG( "55 entl_received ret = %x\n", ret ) ;
		//return 0 ;
	}

	ret = entl_recover( mcn0, &addr ) ;
	if( (ret & ENTL_ACTION_SEND ) != ENTL_ACTION_SEND ){
		ENTL_DEBUG( "61 entl_received ret = %x  expect %x\n", ret , ENTL_ACTION_SEND ) ;
		//return 0 ;		
	}
    //      check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 61", addr,  1,      0,     0,        2,     0, ALO_NOP, 1 ) ;
	daddr1 = addr ;
    set_state(   &reg,  1,  0,  0,  0,     0,  0,  ENTT_NIL,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 61", mcn0, &reg ) ;

	// Missing Packet resend
	ret = entl_received( mcn1, daddr1,       0,         0,           0,            0, &addr, &data ) ;
		ENTL_DEBUG( "entl_received 62 ret = %x  expect %x\n", ret, ENTL_ACTION_SEND ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,     entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  0,  1,     1,  0,  ENTT_NIL,          0,          2 ) ;
    retval &= expect_link_state( "h_l link 59", mcn1, &reg ) ;

    retval &= check_untouched( &mcn0->ao, &src_copy, 100 ) ;  
    retval &= check_untouched( &mcn1->ao, &dst_copy, 100 ) ;



	//dump_state( "mcn0", mcn0 ) ;
	//dump_state( "mcn1", mcn1 ) ;

    return retval ;	
}

int entt_alo_cond_false( entl_state_machine_t *mcn0, entl_state_machine_t *mcn1 ) 
{
	uint64_t addr, daddr0, daddr1 ;
	uint64_t data, data0, data1, wr_data ;
	int retval, ret ;
    ec_link_reg_t reg ;

	entl_state_init( mcn0, &daddr0 ) ;
    //    check_daddr(     str,     daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval = check_daddr( "h_l 1", daddr0,     1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 1", mcn0, &reg ) ;

	entl_state_init( mcn1, &daddr1 ) ;
    //    check_daddr(     str,      daddr, valid, s_or_r, value, tecktack, token,  opcode )
	retval &= check_daddr( "h_l 2", daddr1,     1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
    retval &= expect_link_state( "h_l link 2", mcn1, &reg ) ;
       // entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) 

	//dump_state( "mcn0 0", mcn0 ) ;
	//dump_state( "mcn1 0", mcn1 ) ;

    rand_regs( &mcn0->ao ) ;
    rand_regs( &mcn1->ao ) ;

    copy_regs(&mcn0->ao, &src_copy) ;
    copy_regs(&mcn1->ao, &dst_copy) ;
    wr_data = mcn0->ao.reg[0] ;

    // First packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,       0,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 3", addr,   1,     0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 3", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 4", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
    retval &= expect_link_state( "h_l link 4", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Second packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         1,           0,            0, &addr, &data ) ;
	if( (ret & ENTL_ACTION_SIG_LUP ) != ENTL_ACTION_SIG_LUP ){
		ENTL_DEBUG( "entl_received 4 ret = %x  expect %x\n", ret & ENTL_ACTION_SIG_LUP, ENTL_ACTION_SIG_LUP ) ;
		return 0 ;		
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 5", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0,    0,          1,          0 ) ;
    retval &= expect_link_state( "h_l link 5", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
	if( (ret & ENTL_ACTION_SIG_LUP ) != ENTL_ACTION_SIG_LUP ){
		ENTL_DEBUG( "entl_received 5 ret = %x  expect %x\n", ret & ENTL_ACTION_SIG_LUP, ENTL_ACTION_SIG_LUP ) ;
		return 0 ;		
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 6", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
    set_state(   &reg,  1,  1,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 6", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // Third packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 7", addr,   1,     0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0,    0,          1,          0 ) ;
    retval &= expect_link_state( "h_l link 7", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 8", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 8", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 9", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    0,          2,          0 ) ;
    retval &= expect_link_state( "h_l link 9", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 10", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 10", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // First packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 11", addr,   1,     0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          2,          0 ) ;
    retval &= expect_link_state( "h_l link 11", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 12", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 12", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Second packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 13", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0,    0,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 13", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 14", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 14", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // Third packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 15", addr,   1,     0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0,    0,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 15", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 16", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 16", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 17", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    0,          4,          0 ) ;
    retval &= expect_link_state( "h_l link 17", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 18", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 18", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // First packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 19", addr,   1,     0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          4,          0 ) ;
    retval &= expect_link_state( "h_l link 19", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 20", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 20", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Second packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         0,      ALO_WR | 0x1,            0, &addr, &data ) ; // cond EQ
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 21", addr,  1,      1,     0,        1,     0, ALO_WR | 0x1 , 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff, token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0, ENTT_Ra,          0,           0 ) ;
    retval &= expect_link_state( "h_l link 21", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 22", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff, token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0,       0,          0,           0 ) ;
    retval &= expect_link_state( "h_l link 22", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // Third packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 23", addr,   1,     0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0, ENTT_Ra,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 23", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 24", addr,  1,      0,     0,        1,     0, 0x0000, 0 ) ; // set T flag
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0, ENTT_Rb,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 24", mcn1, &reg ) ;
	daddr0 = addr ;


    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 25", addr,  1,      1,     1,        2,     0, 0x0000, 0 ) ; // set T flag
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0, 	ENTT_Rp,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 25", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 26", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0, ENTT_Rb,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 26", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // Continuous ENTT sequence with token holding

    // First packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 27", addr,   1,     0,     1,        0,    0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0, ENTT_Rp,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 27", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         0,           0,            0, &addr, &data ) ;
	if( (ret & ( ENTL_ACTION_SIG_ALO ) ) != (ENTL_ACTION_SIG_ALO) ) {
		ENTL_DEBUG( "entl_received 27 ret = %x  expect %x\n", (ret  ), (ENTL_ACTION_SIG_ALO) ) ;
		//return 0 ;
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 28", addr,  1,      0,     1,        2,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 28", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Second packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         1,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 29", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     1,  0, ENTT_NIL,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 29", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

	if( (ret & (ENTL_ACTION_SIG_ALO ) ) != (ENTL_ACTION_SIG_ALO) ) {
		ENTL_DEBUG( "entl_received 29 ret = %x  expect %x\n", (ret ), (ENTL_ACTION_SIG_ALO) ) ;
		//return 0 ;
	}

    retval &= check_untouched( &mcn0->ao, &src_copy, 100 ) ;  
    retval &= check_untouched( &mcn1->ao, &dst_copy, 100 ) ;

}

int entt_alo_sequence_p( entl_state_machine_t *mcn0, entl_state_machine_t *mcn1 ) 
{
	uint64_t addr, daddr0, daddr1 ;
	uint64_t data, data0, data1, wr_data ;
	int retval, ret ;
    ec_link_reg_t reg ;


	entl_state_init( mcn0, &daddr0 ) ;
    //    check_daddr(     str,     daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval = check_daddr( "h_l 1", daddr0,     1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 1", mcn0, &reg ) ;

	entl_state_init( mcn1, &daddr1 ) ;
    //    check_daddr(     str,      daddr, valid, s_or_r, value, tecktack, token,  opcode )
	retval &= check_daddr( "h_l 2", daddr1,     1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
    retval &= expect_link_state( "h_l link 2", mcn1, &reg ) ;
       // entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) 

    rand_regs( &mcn0->ao ) ;
    rand_regs( &mcn1->ao ) ;

    copy_regs(&mcn0->ao, &src_copy) ;
    copy_regs(&mcn1->ao, &dst_copy) ;
    wr_data = mcn0->ao.reg[0] ;

	//dump_state( "mcn0 0", mcn0 ) ;
	//dump_state( "mcn1 0", mcn1 ) ;

    // First packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,       0,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 3", addr,   1,     0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 3", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 4", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
    retval &= expect_link_state( "h_l link 4", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Second packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,  data0,         1,           0,            0, &addr, &data ) ;
	if( (ret & ENTL_ACTION_SIG_LUP ) != ENTL_ACTION_SIG_LUP ){
		ENTL_DEBUG( "entl_received ret = %x  expect %x\n", ret & ENTL_ACTION_SIG_LUP, ENTL_ACTION_SIG_LUP ) ;
		return 0 ;		
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 5", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0,    0,          1,          0 ) ;
    retval &= expect_link_state( "h_l link 5", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,  data1,         0,           0,            0, &addr, &data ) ;
	if( (ret & ENTL_ACTION_SIG_LUP ) != ENTL_ACTION_SIG_LUP ){
		ENTL_DEBUG( "entl_received ret = %x  expect %x\n", ret & ENTL_ACTION_SIG_LUP, ENTL_ACTION_SIG_LUP ) ;
		return 0 ;		
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 6", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
    set_state(   &reg,  1,  1,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 6", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // Third packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 7", addr,   1,     0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0,    0,          1,          0 ) ;
    retval &= expect_link_state( "h_l link 7", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 8", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 8", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 9", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    0,          2,          0 ) ;
    retval &= expect_link_state( "h_l link 9", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 10", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 10", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // First packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,    data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 11", addr,   1,     0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          2,          0 ) ;
    retval &= expect_link_state( "h_l link 11", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 12", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 12", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Second packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 13", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0,    0,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 13", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 14", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 14", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // Third packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 15", addr,   1,     0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0,    0,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 15", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 16", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 16", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 17", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    0,          4,          0 ) ;
    retval &= expect_link_state( "h_l link 17", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 18", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 18", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // First packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 19", addr,   1,     0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          4,          0 ) ;
    retval &= expect_link_state( "h_l link 19", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 20", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 20", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Second packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 21", addr,  1,      1,     0,        1,     0, ALO_WR, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff, token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0, ENTT_Ra,          0,           0 ) ;
    retval &= expect_link_state( "h_l link 21", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 22", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff, token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0,       0,          0,           0 ) ;
    retval &= expect_link_state( "h_l link 22", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // Third packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 23", addr,   1,     0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0, ENTT_Ra,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 23", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str,     daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 24", addr,     1,      0,     0,        1,     0, 0x0001, 0 ) ;  // set T flag
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0, ENTT_Rb,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 24", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         0,      ALO_WR,            0, &addr, &data ) ;
	if( (ret & (ENTL_ACTION_SIG_ALO) ) != (ENTL_ACTION_SIG_ALO) ) {
		ENTL_DEBUG( "25 entl_received ret = %x  expect %x\n", (ret), (ENTL_ACTION_SIG_ALO) ) ;
		return 0 ;
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 25", addr,  1,      1,     1,        0,     0, 0x0001, 0 ) ; // set T flag
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     1,  0, 		0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 25", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 26", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0, ENTT_Rb,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 26", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // Continuous ENTT sequence with token holding

    // First packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 27", addr,   1,     0,     1,        0,    1, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 27", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

	//dump_state( "mcn1  27", mcn1 ) ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         0,           0,            0, &addr, &data ) ;

	//dump_state( "mcn1  28", mcn1 ) ;

	if( (ret & ENTL_ACTION_SIG_ALO ) != ENTL_ACTION_SIG_ALO ) {  // no up
		ENTL_DEBUG( "entl_received ret = %x  expect %x\n", (ret ), ENTL_ACTION_SIG_ALO ) ;
		return 0 ;
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 28", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 28", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    retval &= check_untouched( &mcn0->ao, &src_copy, 100 ) ;  
    retval &= check_untouched( &mcn1->ao, &dst_copy, 0 ) ;
    if( mcn1->ao.reg[0] != wr_data ) {
		ENTL_DEBUG( "mc1->ao.reg[0] = %lx  expect %lx\n", mcn1->ao.reg[0], wr_data ) ;
		retval = 0 ;	
    }

    return retval ;	
}

int entt_alo_resend( entl_state_machine_t *mcn0, entl_state_machine_t *mcn1 ) 
{
	uint64_t addr, daddr0, daddr1 ;
	uint64_t data, data0, data1, wr_data ;
	int retval, ret ;
    ec_link_reg_t reg ;


	entl_state_init( mcn0, &daddr0 ) ;
    //    check_daddr(     str,     daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval = check_daddr( "h_l 1", daddr0,     1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 1", mcn0, &reg ) ;

	entl_state_init( mcn1, &daddr1 ) ;
    //    check_daddr(     str,      daddr, valid, s_or_r, value, tecktack, token,  opcode )
	retval &= check_daddr( "h_l 2", daddr1,     1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
    retval &= expect_link_state( "h_l link 2", mcn1, &reg ) ;
       // entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) 

    rand_regs( &mcn0->ao ) ;
    rand_regs( &mcn1->ao ) ;

    copy_regs(&mcn0->ao, &src_copy) ;
    copy_regs(&mcn1->ao, &dst_copy) ;
    wr_data = mcn0->ao.reg[0] ;

	//dump_state( "mcn0 0", mcn0 ) ;
	//dump_state( "mcn1 0", mcn1 ) ;

    // First packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,       0,         1,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 3", addr,   1,     0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  0,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 3", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 4", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
    retval &= expect_link_state( "h_l link 4", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Second packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,  data0,         1,           0,            0, &addr, &data ) ;
	if( (ret & ENTL_ACTION_SIG_LUP ) != ENTL_ACTION_SIG_LUP ){
		ENTL_DEBUG( "entl_received ret = %x  expect %x\n", ret & ENTL_ACTION_SIG_LUP, ENTL_ACTION_SIG_LUP ) ;
		return 0 ;		
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 5", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0,    0,          1,          0 ) ;
    retval &= expect_link_state( "h_l link 5", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,  data1,         0,           0,            0, &addr, &data ) ;
	if( (ret & ENTL_ACTION_SIG_LUP ) != ENTL_ACTION_SIG_LUP ){
		ENTL_DEBUG( "entl_received ret = %x  expect %x\n", ret & ENTL_ACTION_SIG_LUP, ENTL_ACTION_SIG_LUP ) ;
		return 0 ;		
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 6", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
    set_state(   &reg,  1,  1,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 6", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // Third packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 7", addr,   1,     0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0,    0,          1,          0 ) ;
    retval &= expect_link_state( "h_l link 7", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 8", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 8", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 9", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    0,          2,          0 ) ;
    retval &= expect_link_state( "h_l link 9", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 10", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 10", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // First packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,    data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 11", addr,   1,     0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          2,          0 ) ;
    retval &= expect_link_state( "h_l link 11", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 12", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 12", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Second packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 13", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0,    0,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 13", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 14", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 14", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // Third packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 15", addr,   1,     0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0,    0,          3,          0 ) ;
    retval &= expect_link_state( "h_l link 15", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 16", addr,  1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 16", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 17", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    0,          4,          0 ) ;
    retval &= expect_link_state( "h_l link 17", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 18", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 18", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // First packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 19", addr,   1,     0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          4,          0 ) ;
    retval &= expect_link_state( "h_l link 19", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,       0,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 20", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 20", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Second packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 21", addr,  1,      1,     0,        1,     0, ALO_WR, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff, token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0, ENTT_Ra,          0,           0 ) ;
    retval &= expect_link_state( "h_l link 21", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

	// This daddr0 is lost and goes to recover sequence

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 22", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff, token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  0,       0,          0,           0 ) ;
    retval &= expect_link_state( "h_l link 22", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // Third packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr,   valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 23", addr,   1,      0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0, ENTT_Ra,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 23", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

	// Recover sequence

	//dump_state( "mcn1  23", mcn1 ) ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l a", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff, token_bkoff )
    set_state(   &reg,  0,  1,  0,  1,     0,  1,       0,          0,           0 ) ;
    retval &= expect_link_state( "h_l link a", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

	//dump_state( "mcn1  a", mcn1 ) ;

    // Third packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l b", addr,   1,     0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  1,  1,     0,  1, ENTT_Ra,          0,          0 ) ;
    retval &= expect_link_state( "h_l link b", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l c", addr,  1,      1,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff, token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  2,       0,          0,           0 ) ;
    retval &= expect_link_state( "h_l link c", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // Third packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l d", addr,   1,     0,     0,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  2, ENTT_Ra,          0,          0 ) ;
    retval &= expect_link_state( "h_l link d", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l e", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff, token_bkoff )
    set_state(   &reg,  0,  1,  0,  1,     0,  3,       0,          0,           0 ) ;
    retval &= expect_link_state( "h_l link f", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // Third packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l f", addr,   1,     0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  1,  1,     0,  3, ENTT_Ra,          0,          0 ) ;
    retval &= expect_link_state( "h_l link f", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l g", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff, token_bkoff )
    set_state(   &reg,  1,  1,  0,  1,     0,  4,       0,          0,           0 ) ;
    retval &= expect_link_state( "h_l link g", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // Third packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,      ALO_WR,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l h", addr,   1,     0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  1,  1,  1,     0,  3, ENTT_Ra,          0,          0 ) ;
    retval &= expect_link_state( "h_l link h", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;



    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str,     daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 24", addr,     1,      0,     0,        1,     0, 0x0001, 0 ) ;  // set T flag
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  1,  0,  1,  1,     0,  0, ENTT_Rb,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 24", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    // Forth packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr0,   data0,         0,      ALO_WR,            0, &addr, &data ) ;
	if( (ret & (ENTL_ACTION_SIG_ALO) ) != (ENTL_ACTION_SIG_ALO) ) {
		ENTL_DEBUG( "25 entl_received ret = %x  expect %x\n", (ret), (ENTL_ACTION_SIG_ALO) ) ;
		return 0 ;
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 25", addr,  1,      1,     1,        0,     0, 0x0001, 0 ) ; // set T flag
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     1,  0, 		0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 25", mcn0, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 26", addr,  1,      1,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc,    entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  0,  0,  1,     0,  0, ENTT_Rb,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 26", mcn1, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

    // Continuous ENTT sequence with token holding

    // First packet received
    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn0, daddr1,   data1,         0,           0,            0, &addr, &data ) ;
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 27", addr,   1,     0,     1,        0,    1, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 27", mcn0, &reg ) ;
	daddr1 = addr ;
	data1 = data ;

	//dump_state( "mcn1  27", mcn1 ) ;

    //    entl_received( *mcn, d_addr, s_value, ait_queue, ait_command, egress_queue, *addr, *alo_data ) ;
	ret = entl_received( mcn1, daddr0,   data0,         0,           0,            0, &addr, &data ) ;

	//dump_state( "mcn1  28", mcn1 ) ;

	if( (ret & ENTL_ACTION_SIG_ALO ) != ENTL_ACTION_SIG_ALO ) {  // no up
		ENTL_DEBUG( "entl_received ret = %x  expect %x\n", (ret ), ENTL_ACTION_SIG_ALO ) ;
		return 0 ;
	}
    //    check_daddr(     str, daddr, valid, s_or_r, value, tecktack, token,  opcode  )
	retval &= check_daddr( "h_l 28", addr,  1,      0,     1,        0,     0, ALO_NOP, 0 ) ;
	// set_state( reg, sr, rr, tt, tf, token, tc, entt, entt_bkoff,token_bkoff )
    set_state(   &reg,  0,  1,  1,  1,     0,  0,    0,          0,          0 ) ;
    retval &= expect_link_state( "h_l link 28", mcn1, &reg ) ;
	daddr0 = addr ;
	data0 = data ;

    retval &= check_untouched( &mcn0->ao, &src_copy, 100 ) ;  
    retval &= check_untouched( &mcn1->ao, &dst_copy, 0 ) ;
    if( mcn1->ao.reg[0] != wr_data ) {
		ENTL_DEBUG( "mc1->ao.reg[0] = %lx  expect %lx\n", mcn1->ao.reg[0], wr_data ) ;
		retval = 0 ;	
    }

    return retval ;	
}


