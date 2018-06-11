/*
 * Copyright (C) 2018,  Earth Computing Inc.  All rights reserved.
 *
 *
 * @file          ec_control.c
 * @brief         ENTL state machine 
 *
 * Author:        Atsushi Kasuya
 *
 */
 
#include "ec_control.h"

static int debug_flag = 1 ;

static int is_ENTT_queue_full( int queue_size ) {
    return queue_size >= MAX_ENTT_QUEUE_SIZE ;
}

void entl_state_init( __lmem entl_state_machine_t *mcn, uint64_t *addr )
{

	ec_link_field_t fld ;

    mcn->reg._raw = 0 ;

    alo_regs_init( &mcn->ao ) ;

    mcn->recover_count = 0 ;
    mcn->recovered_count = 0 ;
    mcn->s_count = 0 ;
    mcn->r_count = 0 ;
    mcn->entt_count = 0 ;
    mcn->aop_count = 0 ;

    ec_link_send( &mcn->reg, &fld ) ;

    *addr = fld._raw ;

}

int entl_received( __lmem entl_state_machine_t *mcn, uint64_t d_addr, uint64_t s_value, uint32_t ait_queue, uint32_t alo_command, uint32_t egress_queue, uint64_t *addr, uint64_t *alo_data ) 
{
	ec_link_field_t fld_in, fld_out, fld_out2 ;
 	int flag, qfull, ret ;
    int retval = ENTL_ACTION_NOP ;

 	flag = ait_queue || ALO_COMMAND_OPCODE(alo_command) ;
 	qfull = is_ENTT_queue_full( egress_queue ) ;
	fld_in._raw = GET_ECLP_VALUE(d_addr) ;

    if( fld_in.valid == 1 ) {
        if( fld_in.recover == 1 ) {
            mcn->recovered_count++ ;
        }
        if( fld_in.s_or_r == 1 ) {
            mcn->s_count++ ;
        }
        else {
            mcn->r_count++ ;
        }
    }

	ret = ec_link_action( &mcn->reg, fld_in, flag, qfull, &fld_out ) ;
	*addr = fld_out._raw ;  // lsb is the field
    
    retval = ENTL_ACTION_SEND ;

	if( ret & ENTL_ACTION_ENTT_STRT ) {
		if( ALO_COMMAND_OPCODE(alo_command) ) {
            uint32_t t =  alo_initiate( &mcn->ao, ALO_COMMAND_OPCODE(alo_command), ALO_COMMAND_SC(alo_command), alo_data ) ;
            mcn->last_alo_command = alo_command ;
            *addr |= SET_ECLP_ALO(alo_command) ;
            if( ALO_COMMAND_FW(alo_command) ) *addr |= ECLP_FW_MASK ;
            retval = ENTL_ACTION_SEND | ENTL_ACTION_SEND_ALO  ;
		}
		else {
            if( ALO_COMMAND_FW(alo_command) ) *addr |= ECLP_FW_MASK ;
            retval = ENTL_ACTION_SEND | ENTL_ACTION_SEND_AIT  ;
        	mcn->last_alo_command = 0 ;
		}
	}

	if( ret & ENTL_ACTION_ENTT_NEXT ) {
		if( ALO_COMMAND_OPCODE(mcn->last_alo_command) ) {
            int res = alo_next( &mcn->ao, ALO_OPCODE(d_addr), s_value ) ;  // opcode carries the return status
            retval = ENTL_ACTION_SEND ;
            //ENTL_DEBUG( "ENTT_DONE alo_next = %d\n", res ) ;
            if( res ) {
            	*addr |= ( 0x10000 ) ; // T flag set
            	mcn->aop_count++ ;
            }
		}
	}

	if( ret & ENTL_ACTION_ENTT_DONE ) {
		if( ALO_COMMAND_OPCODE(mcn->last_alo_command) ) {
            int res = alo_complete( &mcn->ao ) ;  // opcode carries the return status
            retval = ENTL_ACTION_SEND ;
            //ENTL_DEBUG( "ENTT_DONE alo_complete = %d\n", res ) ;
            if( res ) {
            	retval |= ENTL_ACTION_SIG_ALO ;
            	mcn->aop_count++ ;
            }
		}
		else {
			mcn->entt_count++ ;
            retval = ENTL_ACTION_SEND | ENTL_ACTION_SIG_AIT | ENTL_ACTION_DROP_AIT ;
		}
	}

	if( ret & ENTL_ACTION_ENTT_RECV ) {
        if( ALO_OPCODE(d_addr) == ALO_NOP ) {
            retval = ENTL_ACTION_SEND | ENTL_ACTION_PROC_AIT ;
            mcn->last_alo_command = 0 ;
            mcn->last_d_addr = d_addr ;
        }
        else {
            // Atomic Link Operation 
            uint32_t res = alo_exec( &mcn->ao, ALO_OPCODE(d_addr), ALO_DT(d_addr), s_value, alo_data ) ;
            mcn->last_alo_command = ALO_OPCODE(d_addr) ;
            mcn->last_d_addr = d_addr ;
            if( res ) {
            	*addr |= ( (uint64_t)mcn->ao.return_flag << 16 ) ; // T flag set
            	retval = ENTL_ACTION_SEND | ENTL_ACTION_SEND_ALO ;
                if(d_addr & ECLP_FW_MASK) retval |= ENTL_ACTION_PROPAGATE_AIT ;
            }
            else {
                retval = ENTL_ACTION_SEND | ENTL_ACTION_SEND_ALO_F ;
            }
        }
	}
	if( ret & ENTL_ACTION_ENTT_FNSH ) {
		if( mcn->last_alo_command == 0 ) {
            retval = ENTL_ACTION_SEND | ENTL_ACTION_SIG_AIT | ENTL_ACTION_PROC_AIT ;			
            if(mcn->last_d_addr & ECLP_FW_MASK) retval |= ENTL_ACTION_PROPAGATE_AIT ;
		}
		else {
        	int res = alo_update( &mcn->ao, ALO_OPCODE(d_addr) ) ;  // opcode carries the return status
            //ENTL_DEBUG( "ENTT_DONE alo_update = %d\n", res ) ;
            retval = ENTL_ACTION_SEND ;
            if( res ) {
            	retval |= ENTL_ACTION_SIG_ALO ;
                if(mcn->last_d_addr & ECLP_FW_MASK) retval |= ENTL_ACTION_PROPAGATE_AIT ;
            }
		}
	}
	if( ret & ENTL_ACTION_ENTT_DROP ) {
		if( mcn->last_alo_command == 0 ) {
            retval = ENTL_ACTION_SEND ;			
		}
		else {
			mcn->ao.return_flag = 0 ;
	        alo_complete( &mcn->ao ) ;  // Drop the aop due to conflict
        	retval = ENTL_ACTION_SEND ;
        }
	}
	if( ret & ENTL_ACTION_ENTL_RECOVER ) {
        retval |= ENTL_ACTION_RECOVER ;
	}
	if( ret & ENTL_ACTION_ENTL_LKUP ) {
        retval |= ENTL_ACTION_SIG_LUP ;
	}
	if( ret & ENTL_ACTION_ENTL_LKDN ) {
        retval |= ENTL_ACTION_SIG_LDN ;
	}

 	return retval ;
}

int entl_recover( __lmem entl_state_machine_t *mcn, uint64_t *addr ) 
{
    int retval = ENTL_ACTION_SEND ;
    int ret ;
    ec_link_field_t fld_out ;

    ret = ec_link_recover( &mcn->reg, &fld_out ) ;
	*addr = fld_out._raw ;  // lsb is the field
    mcn->recover_count++ ;

	if( ret & ENTL_ACTION_ENTT_CLER ) {
		if( mcn->last_alo_command == 0 ) {
            retval = ENTL_ACTION_SEND | ENTL_ACTION_CLER_AIT ;			
		}
		else {
        	int res = alo_update( &mcn->ao, 0 ) ;  // clear pending ALO
            retval = ENTL_ACTION_SEND ;
		}
	}

	return retval ;

}



