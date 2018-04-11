/*
 * Copyright (C) 2018,  Earth Computing Inc.  All rights reserved.
 *
 *
 * @file          ec_link.c
 * @brief         ENTL state machine 
 *
 * Author:        Atsushi Kasuya
 *
 */

#ifdef NETRONOME_CHIP
  // Chip environment

#else
#include <stdio.h>

static int debug_flag = 1 ;

#endif

#include "ec_link.h"

void ec_link_init( ec_link_reg_t *reg ) 
{
	reg->_raw = 0 ;
}

void ec_link_send( ec_link_reg_t *reg, ec_link_field_t *ret ) 
{
    ret->_raw = 0 ;

    ret->valid = 1 ; 
    ret->s_or_r = 1 ;  // SR side need to be resend
    ret->value = !reg->sr  ;

}

int ec_link_action( ec_link_reg_t *reg, ec_link_field_t field, int entt, ec_link_field_t *ret ) 
{
	int flag = 0 ;
    ret->_raw = 0 ;

    if( !field.valid ) return flag ;

    if( field.s_or_r ) {  // SR is received
    	ret->valid = 1 ; 
    	ret->s_or_r = 0 ;
        ret->value = reg->rr = field.value ; 
        //if( field.token ) reg->entt_bkoff = 0 ;
        // Token is transfered on RR message
        // Token is always transfered if ENTT queue is empty
        if( !entt & reg->token ) {
            ret->token = reg->token ;
            reg->token = 0 ;
        }
    	switch(reg->entt) {
    		case ENTT_NIL:
    			if( field.tecktack ) {
    				ret->tecktack = 1 ;  // got teck, returning tack
    				reg->entt = ENTT_Rb ;    
    				flag |= ENTL_ACTION_ENTT_RECV ;		
    				reg->token = 0 ;		
    			}
    			break;
    		default:
    			break ; 
    	}
		if ( reg->tt ) {
			if( reg->tc > TC_THRESHHOLD ) {
				flag |= ENTL_ACTION_RECOVER ;
				reg->tc = 0 ;
				reg->tf = 0 ;
			}
			else reg->tc++ ;
		}
		else reg->tc = 0 ;
    	reg->tt = 1 ;
    }
    else {  // RR is received
    	ret->valid = 1 ; 
    	ret->s_or_r = 1 ;
    	reg->sr = field.value ;
    	ret->value = !reg->sr  ;
        reg->token = field.token ;
    	switch(reg->entt) {
    		case ENTT_NIL:
    			if( entt ) {
    				if( reg->token || reg->entt_bkoff > TOKEN_BACKOFF_THRESHHOLD ) {
						ret->tecktack = 1 ;  // Send teck
    					reg->entt = ENTT_Ra ;    				
						flag |= ENTL_ACTION_ENTT_STRT ;
						reg->entt_bkoff = 0 ;
    				}
    				else reg->entt_bkoff++ ;
    			}
    			//else {
    			//	if( reg->token ) {
    			//		reg->token = 0 ;
    			//		ret->token = 1 ;
    			//	}
    			//}
    			break;
    		case ENTT_Ra:
                reg->entt = ENTT_NIL ; // complete ENTT 
    			if( field.tecktack ) {
					flag |= ENTL_ACTION_ENTT_DONE ;
    				if( !entt ) {
    					ret->token = 1 ;
    					reg->token = 0 ;
    				}
    				else {
    					if( reg->entt_bkoff > TOKEN_HOLD_THRESHHOLD ) {
    						ret->token = 1 ;
    						reg->token = 0 ;
    						reg->entt_bkoff = 0 ;
    					}
    					else reg->entt_bkoff++ ;
    				}
    			}
    			else {
					flag |= ENTL_ACTION_ENTT_DROP ;
    				if( reg->token ) {
    					ret->token = 1 ;
    					reg->token = 0 ;
    				}
    				reg->entt_bkoff = 0 ; 
    			}
    			break ; 
    		case ENTT_Rb:
    			reg->entt = ENTT_NIL ; // complete ENTT 
    			flag |= ENTL_ACTION_ENTT_FNSH ;
    			break ; 
    	}
		// error recovery
		if ( reg->tt == 0 ) {
			if( reg->tc > TC_THRESHHOLD ) {
				flag |= ENTL_ACTION_RECOVER ;
				reg->tc = 0 ;
				reg->tf = 0 ;
			}
			else reg->tc++ ;
		}
		else {
			reg->tc = 0 ;
			reg->tf = 1 ;
		}
    	reg->tt = 0 ;
    }
    
	return flag ;
}

void ec_link_resend( ec_link_reg_t *reg, ec_link_field_t field, ec_link_field_t *ret ) 
{
    ret->_raw = 0 ;

    // Resend does not start ENTT, as purpose is to restart ENTL condition
    if( field.s_or_r ) {  // SR is received RR is sent
    	ret->valid = 1 ; 
    	ret->s_or_r = 1 ;  // SR side need to be resend
    	ret->value = !reg->sr  ;
    }
    else {  // _SR is sent
    	ret->valid = 1 ; 
    	ret->s_or_r = 0 ;  // RR side need to be resend
        ret->value = reg->rr ;
    }
}
