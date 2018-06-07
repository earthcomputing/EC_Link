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

int ec_link_recover( ec_link_reg_t *reg, ec_link_field_t *ret ) 
{

    int flag = 0 ;
    ret->_raw = 0 ;

    ret->valid = 1 ; 
    ret->s_or_r = 0 ;
    ret->value = reg->rr ;
    ret->recover = 1 ;
    switch( reg->state ) {
        case ENTT_NIL:
            ret->entt = 0 ;
            break ;
        case ENTT_Rb:
            reg->state = ENTT_NIL ; // cancel ENTT 
            ret->entt = FIELD_ENTA ;
            flag |= ENTL_ACTION_ENTT_CLER ;
            break ;
    }
    return flag ;
}

int ec_link_action( ec_link_reg_t *reg, ec_link_field_t field, int entt, int qfull, ec_link_field_t *ret ) 
{
	int flag = 0 ;
    ret->_raw = 0 ;

    if( !field.valid ) return flag ;

    if( field.s_or_r ) {  // SR is received
    	ret->valid = 1 ; 
    	ret->s_or_r = 0 ;
        ret->value = reg->rr = field.value ; 
        // Token is only transfered on RR message
        // Token is always transfered if ENTT queue is empty
        if( reg->token ) {
            if( !entt ) {
                ret->token = reg->token ; // pass token to another cell
                reg->token = 0 ;
                reg->token_bkoff = 0 ;
            }
            else if( reg->state == ENTT_NIL ) {
                if( reg->token_bkoff > TOKEN_HOLD_THRESHHOLD ) {
                    ret->token = reg->token ;
                    reg->token = 0 ;
                    reg->token_bkoff = 0 ;
                }
            }
        }
    	switch(reg->state) {
    		case ENTT_NIL:
    			if( (field.recover == 0) && (field.entt == FIELD_ENTT) && !qfull ) {
    				ret->entt = FIELD_ENTT ;  // got teck, returning tack
    				reg->state = ENTT_Rb ;    
    				flag |= ENTL_ACTION_ENTT_RECV ;	
    			}
    			break;
            case ENTT_Rb:
                if( field.entt == FIELD_ENTA ) {
                    ret->entt = FIELD_ENTA ;  // got ack, returning ack
                    reg->state = ENTT_NIL ; // complete ENTT 
                    flag |= ENTL_ACTION_ENTT_FNSH ;                    
                    //ENTL_DEBUG( "ENTT_Rb 1\n" ) ;
                }
                else {
                    // error 
                    reg->state = ENTT_NIL ; // cancel ENTT 
                    flag |= (ENTL_ACTION_ENTT_DROP | ENTL_ACTION_ENTL_EROR) ;
                    //ENTL_DEBUG( "ENTT_Rb 2\n" ) ;
                }
                break ; 
    		default:  // reject if it's in Ra state (conflict)
    			break ; 
    	}
		//if ( reg->tt ) {
		//	if( reg->tc > TC_THRESHHOLD ) {
		//		flag |= ENTL_ACTION_ENTL_RECOVER ;
        //        if( reg->tf ) flag |= ENTL_ACTION_ENTL_LKDN ;
		//		reg->tc = 0 ;
		//		reg->tf = 0 ;
		//	}
		//	else reg->tc++ ;
		//}
		//else reg->tc = 0 ;
        //
        //  Only SR side checks TC error
        //
    	reg->tt = 1 ;
    }
    else {  // RR is received
    	ret->valid = 1 ; 
    	ret->s_or_r = 1 ;
    	reg->sr = field.value ;
    	ret->value = !reg->sr  ;
        if( field.token ) {
            reg->token = field.token ;
            reg->entt_bkoff = 0 ;
            reg->token_bkoff = 0 ;
        }
        // error recovery
        if ( reg->tt == 0 ) {
            if( reg->tc > TC_THRESHHOLD ) {
                flag |= ENTL_ACTION_ENTL_RECOVER ;
                if( reg->tf ) flag |= ENTL_ACTION_ENTL_LKDN ;
                reg->tc = 0 ;
                reg->tf = 0 ;
            }
            else reg->tc++ ;
        }
        else {
            reg->tc = 0 ;
            if( reg->tf == 0) flag |= ENTL_ACTION_ENTL_LKUP ;
            reg->tf = 1 ;
        }
        reg->tt = 0 ;
    	switch(reg->state) {
    		case ENTT_NIL:
                if( field.recover == 0 ) 
                {
                    if( entt ) {
                        if( reg->token ) {
                            ret->entt = FIELD_ENTT ;  // Send teck
                            reg->state = ENTT_Ra ;                  
                            flag |= ENTL_ACTION_ENTT_STRT ;
                            reg->token_bkoff++ ;
                        }
                        else if( ++reg->entt_bkoff > TOKEN_BACKOFF_THRESHHOLD )  // should use random to handle conflict 
                        {
                            ret->entt = FIELD_ENTT ;  // Send teck
                            reg->state = ENTT_Ra ;                  
                            flag |= ENTL_ACTION_ENTT_STRT ;
                            reg->entt_bkoff = 0 ;
                        }
                    }
                    //else reg->entt_bkoff++ ;
                }

    			//else {
    			//	if( reg->token ) {
    			//		reg->token = 0 ;
    			//		ret->token = 1 ;
    			//	}
    			//}
    			break;
    		case ENTT_Ra:
    			if( field.recover != 1 ) {
                    if( field.entt == FIELD_ENTL ) // rejected request
                    {
                        reg->state = ENTT_NIL ; // complete ENTT 
                        flag |= ENTL_ACTION_ENTT_DROP ;
                        reg->entt_bkoff++ ;
                    }
                    else if( field.entt == FIELD_ENTT ) {
                        ret->entt = FIELD_ENTA ;  // send ack
                        reg->state = ENTT_Rp ; // complete ENTT 
                        flag |= ENTL_ACTION_ENTT_NEXT ;
                    }
                    else {
                        reg->state = ENTT_NIL ; // impossible sequence 
                        flag |= (ENTL_ACTION_ENTT_DROP | ENTL_ACTION_ENTL_EROR) ;
                    }
    			}
    			else {
                    reg->state = ENTT_NIL ; // terminate ENTT 
					flag |= ENTL_ACTION_ENTT_DROP ;
    			}
                reg->entt_bkoff = 0 ; 
    			break ; 
            case ENTT_Rp:
                reg->state = ENTT_NIL ; // complete ENTT 
                if( field.recover ) {  // recover packet
                    if( field.entt == FIELD_ENTA ) {
                        flag |= ENTL_ACTION_ENTT_DROP ;
                    }
                    else if ( field.entt == FIELD_ENTL ) {
                        flag |= ENTL_ACTION_ENTT_DONE ;
                    }
                    else {
                        // error
                        flag |= (ENTL_ACTION_ENTT_DROP | ENTL_ACTION_ENTL_EROR) ;
                    }
                    field.token = 0 ;  // on recover, token should be lost
                }
                else if( field.entt == FIELD_ENTA ) {
                    flag |= ENTL_ACTION_ENTT_DONE ;
                    reg->token = 1 ;
                }
                else {
                    // error
                    flag |= (ENTL_ACTION_ENTT_DROP | ENTL_ACTION_ENTL_EROR) ;                    
                }
                reg->entt_bkoff = 0 ; 
                break ; 
            default:
                break ; 
    	}
    }
    
	return flag ;
}

