/*---------------------------------------------------------------------------------------------
 *  Copyright © 2017-present Earth Computing Corporation. All rights reserved.
 *  Licensed under the MIT License. See LICENSE.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

#ifdef NETRONOME_HOST

#else

#include <nfp.h>
#include <stdint.h>
#include <stdlib.h>

#include <pkt/pkt.h>

#include <net/csum.h>
#include <net/eth.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/hdr_ext.h>
#include <nfp/mem_atomic.h>
#include <nfp/mem_bulk.h>
#include <nfp6000/nfp_mac.h>
#include <std/reg_utils.h>

#endif  // NETRONOME_HOST

#include "atomic_link_op.h"

void alo_regs_init( __lmem volatile alo_regs_t *alr ) 
{
    int i ;
    for( i = 0 ; i < 32 ; i++) alr->reg[i] = 0 ;
    alr->result_buffer = 0 ;
    alr->flags = 0 ;
    alr->state = 0 ;
}

// Initiate Atomic Link Operation at source
uint32_t alo_initiate( __lmem volatile alo_regs_t *alr, uint16_t opcode, uint16_t sr, uint64_t *s_value ) 
{
    uint16_t type = ALO_TYPE(opcode) ;
    uint32_t retval = ALO_RESULT_ALO_FAIL ;
    alr->sent_flag = 0 ;
    if( type == 0 ) {  // no field ops
    	switch(opcode) {
    		case ALO_NOP:
    			retval = ALO_RESULT_ALO_FAIL ;
    			alr->state = 0 ; // no pending operation
    			break ;
    		case ALO_ADD:
    			alr->sent_value = *s_value = alr->reg[sr] ;
    			alr->state = 0x40000000 | sr ; // set valid flag & keep update destination
                alr->sent_flag = 1 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;
    			break ;
    		case ALO_SUBS:
    			alr->sent_value = *s_value = alr->reg[sr] ;
    			alr->state = 0x40000000 | sr ; // set valid flag & keep update destination
                alr->sent_flag = 1 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;
    			break ;
    		case ALO_SUBD:
    			alr->sent_value = *s_value = alr->reg[sr] ;
    			alr->state = 0x40000000 | sr ; // set valid flag & keep update destination
                alr->sent_flag = 1 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;
    			break ;
    		case ALO_AND:
    			alr->sent_value = *s_value =  alr->reg[sr] ;
    			alr->state = 0x40000000 | sr ; // set valid flag & keep update destination
                alr->sent_flag = 1 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;
    			break ;
    		case ALO_OR:
    			alr->sent_value = *s_value = alr->reg[sr] ;
    			alr->state = 0x40000000 | sr ; // set valid flag & keep update destination
                alr->sent_flag = 1 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;    		
    			break ;
    		case ALO_INC:
    			alr->sent_value = alr->result_buffer = *s_value = alr->reg[sr] + 1 ;
    			alr->state = 0x80000000 | sr ; // set valid flag & keep update destination
                alr->sent_flag = 1 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;    		
    			break ;
    		case ALO_DEC:
    			alr->sent_value = alr->result_buffer = *s_value = alr->reg[sr] - 1 ;
    			alr->state = 0x80000000 | sr ; // set valid flag & keep update destination
                alr->sent_flag = 1 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;    		
    			break ;
    		case ALO_INCS:
    			alr->sent_value = alr->result_buffer = *s_value = alr->reg[sr] + 1 ; // already incremented at source
    			alr->state = 0x80000000 | sr ; // set valid flag & keep update destination
                alr->sent_flag = 1 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;    		
    			break ;
    		case ALO_INCD:
    			alr->sent_value = *s_value = alr->reg[sr] ;
    			alr->state = 0x40000000 | sr ; // set valid flag & keep update destination
                alr->sent_flag = 1 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;    		
    			break ;
    		case ALO_DECS:
    			alr->sent_value = alr->result_buffer = *s_value = alr->reg[sr] - 1 ; // already decremented at source
    			alr->state = 0x80000000 | sr ; // set valid flag & keep update destination
                alr->sent_flag = 1 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;    		
    			break ;
    		case ALO_DECD:
    			alr->sent_value = *s_value = alr->reg[sr] ;
    			alr->state = 0x40000000 | sr ; // set valid flag & keep update destination
                alr->sent_flag = 1 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;    		
    			break ;
    		default:
    			alr->state = 0 ;
    			retval = ALO_RESULT_ALO_FAIL ;
    			break ;
    	}
    }
    else if( type >= 1 && type <= 3 ) { // RD, WR, SWAP use cond field
    	// here the condition matches
    	switch(opcode & ALO_COND_OP_MASK) {
    		case ALO_RD:
    			alr->sent_value = *s_value = alr->reg[sr] ; // data needed for conditional operation
    			alr->state = 0x40000000 | sr ; // set valid flag & keep update destination
                alr->sent_flag = 1 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;    		
    			break ;
    		case ALO_WR:
    			alr->sent_value = *s_value = alr->reg[sr] ;
    			alr->state = 0 ; // no need to update
                alr->sent_flag = 1 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;    		
    			break ;
    		case ALO_SWAP:
    			alr->sent_value = *s_value = alr->reg[sr] ; // data needed for swap & conditional operation
    			alr->state = 0x40000000 | sr ; // set valid flag & keep update destination
    			retval = ALO_RESULT_ALO_SUCCESS ;    		
    			break ;
    	}
    }
    else if( type <= 7 ) {  // BIT ops use bit field
    	uint16_t bit = opcode & ALO_BIT_MASK ;
        switch(opcode & ALO_COND_OP_MASK) {
    		case ALO_BSET:
    			alr->result_buffer = alr->reg[sr] | ((uint64_t)1 << bit) ;
    			alr->state = 0x80000000 | sr ; // set valid flag & keep update destination
    			retval = ALO_RESULT_ALO_SUCCESS ;    		
    			break ;
     		case ALO_BRESET:
                //if( alr->reg[sr] & ((uint64_t)1 << bit) ) {
                    alr->result_buffer = alr->reg[sr] ^ ((uint64_t)1 << bit) ;
                //}
                //else alr->result_buffer = alr->reg[sr] ;
    			alr->state = 0x80000000 | sr ; // set valid flag & keep update destination
    			retval = ALO_RESULT_ALO_SUCCESS ;    		
    			break ;
    		case ALO_BTESTSET:
    			if( (alr->reg[sr] & ((uint64_t)1 << bit)) == 0 ) {
                    //printf( "yes %lx %lx %lx\n", alr->reg[sr], (1 << bit), alr->reg[sr] & (1 << bit)) ;
    				alr->result_buffer = alr->reg[sr] | ((uint64_t)1 << bit) ;
    				alr->state = 0x80000000 | sr ; // set valid flag & keep update destination
    				retval = ALO_RESULT_ALO_SUCCESS ;    		
    			}
    			else {
                    //printf( "no %lx %lx %lx\n", alr->reg[sr], (1 << bit), alr->reg[sr] & (1 << bit)) ;
    				alr->state = 0 ;
    				retval = ALO_RESULT_ALO_FAIL ;
    			}
    			break ;
    		case ALO_BTESTRESET:
    			if( (alr->reg[sr] & ((uint64_t)1 << bit)) != 0 ) {
    				alr->result_buffer = alr->reg[sr] ^ ((uint64_t)1 << bit) ;
    				alr->state = 0x80000000 | sr ; // set valid flag & keep update destination
    				retval = ALO_RESULT_ALO_SUCCESS ;    		
    			}
    			else {
    				alr->state = 0 ;
    				retval = ALO_RESULT_ALO_FAIL ;
    			}
    			break ;
		}
    }
    else {
    	// unsupported opcode
    	alr->state = 0 ;
    	retval = ALO_RESULT_ALO_FAIL ;
    }
    return retval ;

}

int alo_next( __lmem volatile alo_regs_t *alr, uint16_t status, uint64_t r_value ) 
{
    alr->return_flag = (status & 1) ;
    if( alr->state && (status & 1) ) {
        uint16_t sr = alr->state & 0x1f ;
        alr->return_flag |= 1 ;
        if( alr->state & 0x80000000 ) {
            alr->return_value = alr->result_buffer ;
            if( alr->result_buffer == 0 ) alr->return_flag |= 0x2 ;
            else alr->return_flag &= 0xfffffffd ; 
        }
        else if( alr->state & 0x40000000 ) {
            alr->return_value = r_value ;
            if( r_value == 0 ) alr->return_flag |= 0x2 ;
            else alr->return_flag &= 0xfffffffd ; 
        } 
        alr->return_flag &= 0x00ffffff ;
        alr->return_flag |= (0x80000000) | (sr << (26)) ; // 31 : change bit, 30,29,28,27,26 : reg
        return 1 ;
    }
    else alr->return_flag ;   
}

int alo_complete( __lmem volatile alo_regs_t *alr ){
	if( alr->state && (alr->return_flag & 1) ) {
		uint16_t sr = alr->state & 0x1f ;
		alr->reg[sr] = alr->return_value ;
		alr->flags = alr->return_flag ;
		
		alr->flags &= 0x00ffffff ;
		alr->flags |= (0x80000000) | (sr << (26)) ; // 31 : change bit, 30,29,28,27,26 : reg
		alr->state = 0 ;
		return 1 ;
	}
	else {
		alr->state = 0 ;
		return alr->return_flag & 1 ;
	}
}
 
// Execute Atomic Link Operation 
uint32_t alo_exec( __lmem volatile alo_regs_t *alr, uint16_t opcode, uint16_t dt, uint64_t s_value, uint64_t *r_value ) 
{
    uint16_t type = ALO_TYPE(opcode) ;
    uint32_t retval = ALO_RESULT_ALO_FAIL ;
    alr->return_flag = 0x0000 ;
    if( type == 0 ) {  // no field ops
    	switch(opcode) {
    		case ALO_NOP:
    			retval = ALO_RESULT_ALO_FAIL ;
    			alr->state = 0 ; // no pending operation
    			break ;
    		case ALO_ADD:
    			alr->result_buffer = alr->return_value = *r_value = s_value + alr->reg[dt] ;
    			alr->state = 0x80000000 | dt ; // set valid flag & keep update destination
                alr->return_flag = 0x0001 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;
    			break ;
    		case ALO_SUBS:
    			alr->result_buffer = alr->return_value = *r_value = alr->reg[dt] - s_value ;
    			alr->state = 0x80000000 | dt ; // set valid flag & keep update destination
                alr->return_flag = 0x0001 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;
    			break ;
    		case ALO_SUBD:
    			alr->result_buffer = alr->return_value = *r_value = s_value - alr->reg[dt] ;
    			alr->state = 0x80000000 | dt ; // set valid flag & keep update destination
                alr->return_flag = 0x0001 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;
    			break ;
    		case ALO_AND:
    			alr->result_buffer = alr->return_value = *r_value = s_value & alr->reg[dt] ;
    			alr->state = 0x80000000 | dt ; // set valid flag & keep update destination
                alr->return_flag = 0x0001 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;
    			break ;
    		case ALO_OR:
    			alr->result_buffer = alr->return_value = *r_value = s_value | alr->reg[dt] ;
    			alr->state = 0x80000000 | dt ; // set valid flag & keep update destination
                alr->return_flag = 0x0001 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;    		
    			break ;
    		case ALO_INC:
    			alr->result_buffer = alr->return_value = *r_value = alr->reg[dt] + 1 ;
    			alr->state = 0x80000000 | dt ; // set valid flag & keep update destination
                alr->return_flag = 0x0001 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;    		
    			break ;
    		case ALO_DEC:
    			alr->result_buffer = alr->return_value = *r_value = alr->reg[dt] - 1 ;
    			alr->state = 0x80000000 | dt ; // set valid flag & keep update destination
                alr->return_flag = 0x0001 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;    		
    			break ;
    		case ALO_INCS:
    			alr->result_buffer = alr->return_value = *r_value = s_value ; // already incremented at source
    			alr->state = 0x80000000 | dt ; // set valid flag & keep update destination
                alr->return_flag = 0x0001 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;    		
    			break ;
    		case ALO_INCD:
    			alr->result_buffer = alr->return_value = *r_value = alr->reg[dt] + 1 ;
    			alr->state = 0x80000000 | dt ; // set valid flag & keep update destination
                alr->return_flag = 0x0001 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;    		
    			break ;
    		case ALO_DECS:
    			alr->result_buffer = alr->return_value = *r_value = s_value ; // already decremented at source
    			alr->state = 0x80000000 | dt ; // set valid flag & keep update destination
                alr->return_flag = 0x0001 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;    		
    			break ;
    		case ALO_DECD:
    			alr->result_buffer = alr->return_value = *r_value = alr->reg[dt] -1 ;
    			alr->state = 0x80000000 | dt ; // set valid flag & keep update destination
                alr->return_flag = 0x0001 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;    		
    			break ;
    		default:
    			alr->state = 0 ;
                alr->return_flag = 0x0000 ;
    			retval = ALO_RESULT_ALO_FAIL ;
    			break ;
    	}

    }
    else if( type >= 1 && type <= 3 ) { // RD, WR, SWAP use cond field
    	uint16_t cond = opcode & ALO_COND_MASK ;
    	int res = 0 ;
    	switch( cond ) {
    		case ALO_COND_ALLWAYS:
    			res = 1 ;
    			break ;
    		case ALO_COND_EQ:
    			res = ( alr->reg[dt] == s_value );
    			break ;
    		case ALO_COND_LT:
    			res = ( alr->reg[dt] < s_value );
    			break ;
    		case ALO_COND_LE:
    			res = ( alr->reg[dt] <= s_value );
    			break ;
    		case ALO_COND_GT:
    			res = ( alr->reg[dt] > s_value );
    			break ;
    		case ALO_COND_GE:
    			res = ( alr->reg[dt] >= s_value );
    			break ;
    	}
    	if( res == 0 ) {
    		alr->state = 0 ;
            alr->return_flag = 0x0000 ;
    		retval = ALO_RESULT_ALO_FAIL ;
    		return retval ;
    	}
    	// here the condition matches
        retval = ALO_RESULT_ALO_SUCCESS ;
    	switch(opcode & ALO_COND_OP_MASK) {
    		case ALO_RD:
    			alr->return_value = *r_value = alr->reg[dt] ;
                alr->return_flag = 0x0001 ;
    			alr->state = 0 ;
    			break ;
    		case ALO_WR:
    			alr->result_buffer = alr->return_value = *r_value = s_value ;
                alr->return_flag = 0x0001 ;
    			alr->state = 0x80000000 | dt ; // set valid flag & keep update destination
    			retval = ALO_RESULT_ALO_SUCCESS ;    		
    			break ;
    		case ALO_SWAP:
    			alr->return_value = *r_value = alr->reg[dt] ;
    			alr->result_buffer = s_value ;
                alr->return_flag = 0x0001 ;
    			alr->state = 0x80000000 | dt ; // set valid flag & keep update destination
    			retval = ALO_RESULT_ALO_SUCCESS ;    		
    			break ;
    	}
    }
    else if( type <= 7 ) {  // BIT ops use bit field
    	uint16_t bit = opcode & ALO_BIT_MASK ;
        switch(opcode & ALO_COND_OP_MASK) {
    		case ALO_BSET:
    			alr->return_value = *r_value = s_value ;
    			alr->result_buffer = alr->reg[dt] | ((uint64_t)1 << bit) ;
    			alr->state = 0x80000000 | dt ; // set valid flag & keep update destination
                alr->return_flag = 0x0001 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;    		
    			break ;
     		case ALO_BRESET:
    			alr->return_value = *r_value = s_value ;
                //if( alr->reg[dt] & ((uint64_t)1 << bit) )
    			     alr->result_buffer = alr->reg[dt] ^ ((uint64_t)1 << bit) ;
                //else alr->result_buffer = alr->reg[dt] ;
    			alr->state = 0x80000000 | dt ; // set valid flag & keep update destination
                alr->return_flag = 0x0001 ;
    			retval = ALO_RESULT_ALO_SUCCESS ;    		
    			break ;
    		case ALO_BTESTSET:
    			alr->return_value = *r_value = s_value ;
    			if( (alr->reg[dt] & ((uint64_t)1 << bit)) == 0 ) {
                    //printf( "yes %lx %lx %lx\n", alr->reg[dt], ((uint64_t)1 << bit), alr->reg[dt] & ((uint64_t)1 << bit)) ;
    				alr->result_buffer = alr->reg[dt] | ((uint64_t)1 << bit) ;
    				alr->state = 0x80000000 | dt ; // set valid flag & keep update destination
                    alr->return_flag = 0x0001 ;
    				retval = ALO_RESULT_ALO_SUCCESS ;    		
    			}
    			else {
                    //printf( "no %lx %lx %lx\n", alr->reg[dt], ((uint64_t)1 << bit), alr->reg[dt] & ((uint64_t)1 << bit)) ;
    				alr->state = 0 ;
                    alr->return_flag = 0x0000 ;
    				retval = ALO_RESULT_ALO_FAIL ;
    			}
    			break ;
    		case ALO_BTESTRESET:
    			alr->return_value = *r_value = s_value ;
    			if( (alr->reg[dt] & ((uint64_t)1 << bit)) != 0 ) {
    				alr->result_buffer = alr->reg[dt] ^ ((uint64_t)1 << bit) ;
    				alr->state = 0x80000000 | dt ; // set valid flag & keep update destination
                    alr->return_flag = 0x0001 ;
    				retval = ALO_RESULT_ALO_SUCCESS ;    		
    			}
    			else {
    				alr->state = 0 ;
                    alr->return_flag = 0x0000 ;
    				retval = ALO_RESULT_ALO_FAIL ;
    			}
    			break ;
		}
    }
    else {
    	// unsupported opcode
    	alr->state = 0 ;
        alr->return_flag = 0x0000 ;
    	retval = ALO_RESULT_ALO_FAIL ;
    }
    return retval ;
}

int alo_state(__lmem volatile alo_regs_t *alr ) 
{
    if( alr->state ) return 1 ;
    return 0 ;
}

int alo_update( __lmem volatile alo_regs_t *alr, uint16_t status ){
	if( alr->state && (status & 1) ) {
		uint16_t dt = alr->state & 0x1f ;
		alr->reg[dt] = alr->result_buffer ;
		alr->flags |= 1 ;
		// set zero bit for the result
		if( alr->result_buffer == 0 ) alr->flags |= 0x2 ;
		else alr->flags &= 0xfffffffd ; 
        alr->flags &= 0x00ffffff ;
		alr->flags |= (0x80000000) | (dt << (26)) ; // 31 : change bit, 30,29,28,27,26 : reg
		alr->state = 0 ;
		return 1 ; 
	}
	else {
        alr->flags &= 0x00ffffff ;
		alr->state = 0 ;
		return 0 ;
	}
}


