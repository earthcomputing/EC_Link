/*
 * Copyright (C) 2017,2018,  Earth Computing Inc.  All rights reserved.
 *
 *
 * @file          alo_tester.c
 * @brief         Atomic Link Operation Tester
 *
 * Author:        Atsushi Kasuya
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "cyc_random.h"

#include "alo_tester.h"
#define ENTL_DEBUG(fmt, args...) if(debug_flag) printf( fmt, ## args )

static alo_regs_t src_copy ;
static alo_regs_t dst_copy ;

static debug_flag = 0 ;

void dump_regs ( alo_regs_t *eng, char *name ) {
    int i ;
    if( name ) ENTL_DEBUG( "engine %s :\n", name) ;
    for( i = 0 ; i < 32 ; i++ ) {
        ENTL_DEBUG( "  reg[%d] : %lx\n", i, eng->reg[i] ) ;
    }
    ENTL_DEBUG( "  result_buffer: %lx flags: %x state: %x \n", eng->result_buffer, eng->flags, eng->state) ;
    ENTL_DEBUG( "  return_value: %lx return_flag: %x state: %x \n", eng->return_value, eng->return_flag ) ;
}

void clear_regs( alo_regs_t *eng ) {
    int i ;
    for( i = 0 ; i < 32 ; i++ ) eng->reg[i] = 0 ;
}


void rand_regs( alo_regs_t *eng )  {
    int i ;
    uint64_t val ;
    for( i = 0 ; i < 32 ; i++ ) {
        val = mt_urandom(0) ; 
        val = (val<<32) | mt_urandom(0) ;
        eng->reg[i] = val ;
    }
}

void copy_regs( alo_regs_t *source, alo_regs_t *destination ) 
{
    int i ;
    for( i = 0 ; i < 32 ; i++ ) destination->reg[i] = source->reg[i]  ;
}

int check_untouched( alo_regs_t *source, alo_regs_t *destination, uint16_t except ) 
{
    int i ;
    for( i = 0 ; i < 32 ; i++ ) {
        if( i != except && destination->reg[i] != source->reg[i] ) {
            ENTL_DEBUG( "  reg[%d] : source %lx != destination %lx \n", i, source->reg[i], destination->reg[i] ) ;
            return 0 ;
        }
    }
    return 1 ;
}

int check_cond( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt, uint16_t opcode )
{
    uint64_t s_value, d_value ;
    uint16_t cond = opcode & ALO_COND_MASK ;
    s_value = se->reg[sr] ;
    d_value = de->reg[dt] ;
    switch( cond ) {
        case ALO_COND_ALLWAYS:
            return 1 ;
            break ;
        case ALO_COND_EQ:
            if ( d_value == s_value ) return 1 ;
            break ;
        case ALO_COND_LT:
            if ( d_value < s_value ) return 1 ;
            break ;
        case ALO_COND_LE:
            if ( d_value <= s_value ) return 1 ;
            break ;
        case ALO_COND_GT:
            if ( d_value > s_value ) return 1 ;
            break ;
        case ALO_COND_GE:
            if ( d_value >= s_value ) return 1 ;
            break ;
    }
    return 0 ;
}

int alo_exec_ops( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt, uint16_t opcode ) 
{
    uint64_t s_value, d_value ;
    uint32_t res0, res1, res2, res3 ;

    s_value = 0xbadbeef ;
    res0 = alo_initiate( se, opcode, sr, &s_value ) ;
    ENTL_DEBUG( "alo_initiate sr %lx opcode:%x s_value:%lx state %x res:%x \n", se->reg[sr], opcode, s_value, se->state, res0 ) ;

    if( res0 ) {
        d_value = 0xbadbeef ;
        res1 = alo_exec( de, opcode, dt, s_value, &d_value ) ;
        ENTL_DEBUG( "alo_exec dt %lx opcode:%x s_value:%lx d_value:%lx state %x res:%x \n", de->reg[dt], opcode, s_value, d_value, de->state, res1) ;
        if( de->return_flag && de->return_value != d_value ) {
            ENTL_DEBUG( "alo_exec return value failed\n" ) ;
            return 0 ;
        }
        res2 = alo_next( se, de->return_flag, de->return_value ) ;
        res2 = alo_complete( se ) ;
        ENTL_DEBUG( "alo_complete sr %lx opcode:%x d_value:%lx flag:%x res:%x \n", se->reg[sr], opcode, d_value, se->flags, res2) ;

        ENTL_DEBUG( "before alo_update dt %lx opcode:%x state %x flag:%x res:%x \n", de->reg[dt], opcode, de->state, de->flags, res2) ;
        res3 = alo_update( de, res2 ) ;
        ENTL_DEBUG( "alo_update dr %lx opcode:%x d_value:%lx flag:%x res:%x \n", de->reg[sr], opcode, d_value, de->flags, res3) ;

        return res1 & res2 & res3 ;
    }
    return 1 ;

}

int alo_check_result( char *name, alo_regs_t *eng, uint16_t reg, uint64_t expect , uint32_t expect_flags ) 
{
    if( eng->reg[reg] != expect ) {
        ENTL_DEBUG( "alo_check_result %s reg %d value:%lx != expect %lx \n", name, reg, eng->reg[reg], expect ) ;
        return 0 ;
    }
    if( eng->flags != expect_flags ) {
        ENTL_DEBUG( "alo_check_result %s flags:%x != expect %x \n", name, eng->flags, expect_flags ) ;
        return 0 ;
    }
    return 1 ;
}

int get_bit( alo_regs_t *eng, uint16_t reg, uint16_t bit ) 
{
    return (eng->reg[reg]>>bit) & 1 ;
}


int alo_nop_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt ) 
{
    int ret0, ret1 ;
    uint16_t opcode = ALO_NOP ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = check_untouched( se, &src_copy, 100 ) ;  // 100 means no exception
    ret1 = check_untouched( de, &dst_copy, 100 ) ;

    return (ret0 && ret1) ;

}

// 31 : change bit, 30,29,28,27,26 : reg
#define FLAG_REG_CHANGE(x) (0x80000000 | (x << (26)))

int alo_add_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt ) 
{
    int ret0, ret1 ;
    uint64_t value ;
    uint16_t opcode = ALO_ADD ;
    uint32_t flag0, flag1 ;

    ENTL_DEBUG( "alo_add_test (1) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    // test as is 
    value = se->reg[sr] + de->reg[dt] ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value == 0 ) {
        flag0 |= 2 ; 
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;

    // zero add to test zero flag set
    ENTL_DEBUG( "alo_add_test (2) \n" ) ;
    value = se->reg[sr] = de->reg[dt] = 0 ;
    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 3 ;
    flag1 |= 3 ;

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se2", se, sr, value , flag0 ) ;
    ret1 = alo_check_result( "de2", de, dt, value , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;

    // carry to zero flag set
    ENTL_DEBUG( "alo_add_test (3) \n" ) ;
    se->reg[sr] = 1 ;
    de->reg[dt] = 0xffffffffffffffff ;
    value = se->reg[sr] + de->reg[dt] ;
    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 3 ;
    flag1 |= 3 ;

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se3", se, sr, value , flag0 ) ;
    ret1 = alo_check_result( "de3", de, dt, value , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;

    //again with other side
    ENTL_DEBUG( "alo_add_test (4) \n" ) ;
    se->reg[sr] = 0xffffffffffffffff ;
    de->reg[dt] = 1 ;
    value = se->reg[sr] + de->reg[dt] ;
    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 3 ;
    flag1 |= 3 ;

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se4", se, sr, value , flag0 ) ;
    ret1 = alo_check_result( "de4", de, dt, value , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ; 
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ENTL_DEBUG( "alo_add_test (5) \n" ) ;
    // check non zero result
    se->reg[sr] = 0 ;
    de->reg[dt] = 1 ;
    value = se->reg[sr] + de->reg[dt] ;
    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se5", se, sr, value , flag0 ) ;
    ret1 = alo_check_result( "de5", de, dt, value , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;    

    // again on other side
    ENTL_DEBUG( "alo_add_test (6) \n" ) ;
    se->reg[sr] = 1 ;
    de->reg[dt] = 0 ;
    value = se->reg[sr] + de->reg[dt] ;
    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se6", se, sr, value , flag0 ) ;
    ret1 = alo_check_result( "de6", de, dt, value , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;    

    return 1 ;

}

int alo_subs_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt ) 
{
    int ret0, ret1 ;
    uint64_t value ;
    uint16_t opcode = ALO_SUBS ;
    uint32_t flag0, flag1 ;

    //debug_flag = 1 ;

    ENTL_DEBUG( "alo_subs_test (1) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    // test as is 
    value = de->reg[dt] - se->reg[sr] ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value == 0 ) {
        flag0 |= 2 ; 
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ENTL_DEBUG( "alo_subs_test (2) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    // zero flag test
    value = mt_urandom(0) ; ;
    value = (value<<32) | mt_urandom(0) ;
    de->reg[dt] = se->reg[sr] = value ;
    value = 0 ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 3 ;
    flag1 |= 3 ;

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;
    return 1 ;

}

int alo_subd_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt ) 
{
    int ret0, ret1 ;
    uint64_t value ;
    uint16_t opcode = ALO_SUBD ;
    uint32_t flag0, flag1 ;

    ENTL_DEBUG( "alo_subd_test (1) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    // test as is 
    value = se->reg[sr] - de->reg[dt] ;
    ENTL_DEBUG( "alo_subd_test (1) s %lx - d %lx = %lx\n", se->reg[sr], de->reg[dt], value ) ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value == 0 ) {
        flag0 |= 2 ; 
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ENTL_DEBUG( "alo_subd_test (2) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    // zero flag test
    value = mt_urandom(0) ; ;
    value = (value<<32) | mt_urandom(0) ;
    de->reg[dt] = se->reg[sr] = value ;
    value = 0 ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 3 ;
    flag1 |= 3 ;

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;    
    return 1 ;
}

int alo_and_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt ) 
{
    int ret0, ret1 ;
    uint64_t value ;
    uint16_t opcode = ALO_AND ;
    uint32_t flag0, flag1 ;

    ENTL_DEBUG( "alo_and_test (1) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    // test as is 
    value = se->reg[sr] & de->reg[dt] ;
    ENTL_DEBUG( "alo_and_test (1) s %lx & d %lx = %lx\n", se->reg[sr], de->reg[dt], value ) ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value == 0 ) {
        flag0 |= 2 ; 
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;

    // zero test 

    ENTL_DEBUG( "alo_and_test (2) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    se->reg[sr] = de->reg[dt] ^ 0xffffffffffffffff ;
    value = se->reg[sr] & de->reg[dt] ;
    ENTL_DEBUG( "alo_and_test (2) s %lx & d %lx = %lx\n", se->reg[sr], de->reg[dt], value ) ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value == 0 ) {
        flag0 |= 2 ; 
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;

    return 1 ;    

}


int alo_or_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt ) 
{
    int ret0, ret1 ;
    uint64_t value ;
    uint16_t opcode = ALO_OR ;
    uint32_t flag0, flag1 ;

    // debug_flag = 1 ;

    ENTL_DEBUG( "alo_or_test (1) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    // test as is 
    value = se->reg[sr] | de->reg[dt] ;
    ENTL_DEBUG( "alo_or_test (1) s %lx & d %lx = %lx\n", se->reg[sr], de->reg[dt], value ) ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value == 0 ) {
        flag0 |= 2 ; 
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;

    // full test 

    ENTL_DEBUG( "alo_and_test (2) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    se->reg[sr] = de->reg[dt] ^ 0xffffffffffffffff ;
    value = se->reg[sr] | de->reg[dt] ;
    ENTL_DEBUG( "alo_and_test (2) s %lx & d %lx = %lx\n", se->reg[sr], de->reg[dt], value ) ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value == 0 ) {
        flag0 |= 2 ; 
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;

    // zero test
    ENTL_DEBUG( "alo_and_test (3) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    se->reg[sr] = de->reg[dt] = 0 ;
    value = se->reg[sr] | de->reg[dt] ;
    ENTL_DEBUG( "alo_and_test (3) s %lx & d %lx = %lx\n", se->reg[sr], de->reg[dt], value ) ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value == 0 ) {
        flag0 |= 2 ; 
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;    

    return 1 ;        
}

int alo_inc_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt ) 
{
    int ret0, ret1 ;
    uint64_t value0, value1 ;
    uint16_t opcode = ALO_INC ;
    uint32_t flag0, flag1 ;

    //debug_flag = 1 ;

    ENTL_DEBUG( "alo_inc_test (1) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    // test as is 
    value0 = se->reg[sr] + 1 ;
    value1 = de->reg[dt] + 1 ;
    ENTL_DEBUG( "alo_inc_test (1) s %lx d %lx ->  %lx %lx\n", se->reg[sr], de->reg[dt], value0, value1 ) ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value0 == 0 ) {
        flag0 |= 2 ; 
    }
    if( value1 == 0 ) {
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;

    // sr zero test
    ENTL_DEBUG( "alo_inc_test (2) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    se->reg[sr] = 0xffffffffffffffff ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    // test as is 
    value0 = se->reg[sr] + 1 ;
    value1 = de->reg[dt] + 1 ;
    ENTL_DEBUG( "alo_inc_test (1) s %lx d %lx ->  %lx %lx\n", se->reg[sr], de->reg[dt], value0, value1 ) ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value0 == 0 ) {
        flag0 |= 2 ; 
    }
    if( value1 == 0 ) {
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;

    // dt zero test
    ENTL_DEBUG( "alo_inc_test (3) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    se->reg[sr] = mt_urandom(0) ;
    de->reg[dt] = 0xffffffffffffffff ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    // test as is 
    value0 = se->reg[sr] + 1 ;
    value1 = de->reg[dt] + 1 ;
    ENTL_DEBUG( "alo_inc_test (3) s %lx d %lx ->  %lx %lx\n", se->reg[sr], de->reg[dt], value0, value1 ) ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value0 == 0 ) {
        flag0 |= 2 ; 
    }
    if( value1 == 0 ) {
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;
    return 1 ;        

}

int alo_dec_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt ) 
{
    int ret0, ret1 ;
    uint64_t value0, value1 ;
    uint16_t opcode = ALO_DEC ;
    uint32_t flag0, flag1 ;

    //debug_flag = 1 ;

    ENTL_DEBUG( "alo_dec_test (1) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    // test as is 
    value0 = se->reg[sr] - 1 ;
    value1 = de->reg[dt] - 1 ;
    ENTL_DEBUG( "alo_dec_test (1) s %lx d %lx ->  %lx %lx\n", se->reg[sr], de->reg[dt], value0, value1 ) ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value0 == 0 ) {
        flag0 |= 2 ; 
    }
    if( value1 == 0 ) {
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    // sr zero test
    ENTL_DEBUG( "alo_dec_test (2) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    se->reg[sr] = 1 ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;


    // test as is 
    value0 = se->reg[sr] - 1 ;
    value1 = de->reg[dt] - 1 ;
    ENTL_DEBUG( "alo_dec_test (1) s %lx d %lx ->  %lx %lx\n", se->reg[sr], de->reg[dt], value0, value1 ) ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value0 == 0 ) {
        flag0 |= 2 ; 
    }
    if( value1 == 0 ) {
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;

    // dt zero test
    ENTL_DEBUG( "alo_dec_test (3) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    se->reg[sr] = mt_urandom(0) ;
    de->reg[dt] = 1 ;
    // test as is 
    value0 = se->reg[sr] - 1 ;
    value1 = de->reg[dt] - 1 ;
    ENTL_DEBUG( "alo_dec_test (3) s %lx d %lx ->  %lx %lx\n", se->reg[sr], de->reg[dt], value0, value1 ) ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value0 == 0 ) {
        flag0 |= 2 ; 
    }
    if( value1 == 0 ) {
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;

    return 1 ;        

}

int alo_incs_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt ) 
{
    int ret0, ret1 ;
    uint64_t value0, value1 ;
    uint16_t opcode = ALO_INCS ;
    uint32_t flag0, flag1 ;

    //debug_flag = 1 ;

    ENTL_DEBUG( "alo_incs_test (1) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    // test as is 
    value0 = se->reg[sr] + 1 ;
    value1 = se->reg[sr] + 1 ;
    ENTL_DEBUG( "alo_incs_test (1) s %lx d %lx ->  %lx %lx\n", se->reg[sr], de->reg[dt], value0, value1 ) ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value0 == 0 ) {
        flag0 |= 2 ; 
    }
    if( value1 == 0 ) {
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;

    // sr zero test
    ENTL_DEBUG( "alo_incs_test (2) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    se->reg[sr] = 0xffffffffffffffff ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    // test as is 
    value0 = se->reg[sr] + 1 ;
    value1 = se->reg[sr] + 1 ;
    ENTL_DEBUG( "alo_incs_test (1) s %lx d %lx ->  %lx %lx\n", se->reg[sr], de->reg[dt], value0, value1 ) ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value0 == 0 ) {
        flag0 |= 2 ; 
    }
    if( value1 == 0 ) {
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;

    // dt zero test
    ENTL_DEBUG( "alo_incs_test (3) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    se->reg[sr] = mt_urandom(0) ;
    de->reg[dt] = 0xffffffffffffffff ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    // test as is 
    value0 = se->reg[sr] + 1 ;
    value1 = se->reg[sr] + 1 ;
    ENTL_DEBUG( "alo_incs_test (3) s %lx d %lx ->  %lx %lx\n", se->reg[sr], de->reg[dt], value0, value1 ) ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value0 == 0 ) {
        flag0 |= 2 ; 
    }
    if( value1 == 0 ) {
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;
    return 1 ;        

}

int alo_incd_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt ) 
{
    int ret0, ret1 ;
    uint64_t value0, value1 ;
    uint16_t opcode = ALO_INCD ;
    uint32_t flag0, flag1 ;

    //debug_flag = 1 ;

    ENTL_DEBUG( "alo_incd_test (1) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    // test as is 
    value0 = de->reg[dt] + 1 ;
    value1 = de->reg[dt] + 1 ;
    ENTL_DEBUG( "alo_incd_test (1) s %lx d %lx ->  %lx %lx\n", se->reg[sr], de->reg[dt], value0, value1 ) ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value0 == 0 ) {
        flag0 |= 2 ; 
    }
    if( value1 == 0 ) {
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;

    // sr zero test
    ENTL_DEBUG( "alo_incd_test (2) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    se->reg[sr] = 0xffffffffffffffff ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    // test as is 
    value0 = de->reg[dt] + 1 ;
    value1 = de->reg[dt] + 1 ;
    ENTL_DEBUG( "alo_incd_test (1) s %lx d %lx ->  %lx %lx\n", se->reg[sr], de->reg[dt], value0, value1 ) ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value0 == 0 ) {
        flag0 |= 2 ; 
    }
    if( value1 == 0 ) {
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;

    // dt zero test
    ENTL_DEBUG( "alo_incd_test (3) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    se->reg[sr] = mt_urandom(0) ;
    de->reg[dt] = 0xffffffffffffffff ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    // test as is 
    value0 = de->reg[dt] + 1 ;
    value1 = de->reg[dt] + 1 ;
    ENTL_DEBUG( "alo_incd_test (3) s %lx d %lx ->  %lx %lx\n", se->reg[sr], de->reg[dt], value0, value1 ) ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value0 == 0 ) {
        flag0 |= 2 ; 
    }
    if( value1 == 0 ) {
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;
    return 1 ;        

}

int alo_decs_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt ) 
{
    int ret0, ret1 ;
    uint64_t value0, value1 ;
    uint16_t opcode = ALO_DECS ;
    uint32_t flag0, flag1 ;

    //debug_flag = 1 ;

    ENTL_DEBUG( "alo_decs_test (1) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    // test as is 
    value0 = se->reg[sr] - 1 ;
    value1 = se->reg[sr] - 1 ;
    ENTL_DEBUG( "alo_decs_test (1) s %lx d %lx ->  %lx %lx\n", se->reg[sr], de->reg[dt], value0, value1 ) ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value0 == 0 ) {
        flag0 |= 2 ; 
    }
    if( value1 == 0 ) {
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    // sr zero test
    ENTL_DEBUG( "alo_decs_test (2) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    se->reg[sr] = 1 ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;


    // test as is 
    value0 = se->reg[sr] - 1 ;
    value1 = se->reg[sr] - 1 ;
    ENTL_DEBUG( "alo_decs_test (1) s %lx d %lx ->  %lx %lx\n", se->reg[sr], de->reg[dt], value0, value1 ) ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value0 == 0 ) {
        flag0 |= 2 ; 
    }
    if( value1 == 0 ) {
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;

    // dt zero test
    ENTL_DEBUG( "alo_decs_test (3) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    se->reg[sr] = mt_urandom(0) ;
    de->reg[dt] = 1 ;
    // test as is 
    value0 = se->reg[sr] - 1 ;
    value1 = se->reg[sr] - 1 ;
    ENTL_DEBUG( "alo_decs_test (3) s %lx d %lx ->  %lx %lx\n", se->reg[sr], de->reg[dt], value0, value1 ) ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value0 == 0 ) {
        flag0 |= 2 ; 
    }
    if( value1 == 0 ) {
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;

    return 1 ;        

}

int alo_decd_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt ) 
{
    int ret0, ret1 ;
    uint64_t value0, value1 ;
    uint16_t opcode = ALO_DECD ;
    uint32_t flag0, flag1 ;

    //debug_flag = 1 ;

    ENTL_DEBUG( "alo_decs_test (1) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    // test as is 
    value0 = de->reg[dt] - 1 ;
    value1 = de->reg[dt] - 1 ;
    ENTL_DEBUG( "alo_decs_test (1) s %lx d %lx ->  %lx %lx\n", se->reg[sr], de->reg[dt], value0, value1 ) ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value0 == 0 ) {
        flag0 |= 2 ; 
    }
    if( value1 == 0 ) {
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    // sr zero test
    ENTL_DEBUG( "alo_decs_test (2) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    se->reg[sr] = 1 ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;


    // test as is 
    value0 = de->reg[dt] - 1 ;
    value1 = de->reg[dt] - 1 ;
    ENTL_DEBUG( "alo_decs_test (1) s %lx d %lx ->  %lx %lx\n", se->reg[sr], de->reg[dt], value0, value1 ) ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value0 == 0 ) {
        flag0 |= 2 ; 
    }
    if( value1 == 0 ) {
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;

    // dt zero test
    ENTL_DEBUG( "alo_decs_test (3) sr %04x dt %04x %x %x\n", sr, dt, FLAG_REG_CHANGE(sr), FLAG_REG_CHANGE(dt) ) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    se->reg[sr] = mt_urandom(0) ;
    de->reg[dt] = 1 ;
    // test as is 
    value0 = de->reg[dt] - 1 ;
    value1 = de->reg[dt] - 1 ;
    ENTL_DEBUG( "alo_decs_test (3) s %lx d %lx ->  %lx %lx\n", se->reg[sr], de->reg[dt], value0, value1 ) ;
    flag0 = FLAG_REG_CHANGE(sr) ;
    flag1 = FLAG_REG_CHANGE(dt) ;
    flag0 |= 1 ;
    flag1 |= 1 ;
    if( value0 == 0 ) {
        flag0 |= 2 ; 
    }
    if( value1 == 0 ) {
        flag1 |= 2 ; 
    }

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;

    return 1 ;        

}

int test_values( uint64_t sd, uint64_t dd, uint16_t cond) {
    switch(cond){
        case ALO_COND_ALLWAYS: return 1 ;
        case ALO_COND_EQ: return (sd == dd) ;
        case ALO_COND_LT: return (dd < sd) ;
        case ALO_COND_LE: return (dd <= sd) ;
        case ALO_COND_GT: return (dd > sd) ;
        case ALO_COND_GE: return (dd >= sd) ;
    }
    return 0 ;
}

uint64_t true_dt_values( uint64_t sd, uint16_t cond) {
    switch(cond){
        case ALO_COND_ALLWAYS: return mt_urandom(0) ;
        case ALO_COND_EQ: return sd ;
        case ALO_COND_LT: return sd-1 ;
        case ALO_COND_LE: return sd ;
        case ALO_COND_GT: return sd+1 ;
        case ALO_COND_GE: return sd ;
    }
    return 0 ;
}

uint64_t false_dt_values( uint64_t sd, uint16_t cond) {
    switch(cond){
        case ALO_COND_ALLWAYS: return mt_urandom(0) ;
        case ALO_COND_EQ: return sd-1 ;
        case ALO_COND_LT: return sd ;
        case ALO_COND_LE: return sd+1 ;
        case ALO_COND_GT: return sd ;
        case ALO_COND_GE: return sd-1 ;
    }
    return 0 ;
}

int alo_rd_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt, uint16_t cond ) 
{
    int ret0, ret1 ;
    uint64_t value0, value1 ;
    uint16_t opcode = ALO_RD | cond ;
    uint32_t flag0, flag1 ;
    int cc ;

    //debug_flag = 1 ;

    ENTL_DEBUG( "alo_rd_test (1) sr %04x dt %04x cond %d \n", sr, dt, cond ) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    se->flags = 0 ;
    de->flags = 0 ;

    cc = test_values( se->reg[sr], de->reg[dt], cond) ;
    // test as is 
    value1 = de->reg[dt] ;
    ENTL_DEBUG( "alo_rd_test (1) cond %d s %lx d %lx ->  %lx %lx\n", cond, se->reg[sr], de->reg[dt], value0, value1 ) ;
    if( cc ) {
        value0 = de->reg[dt] ;
        flag0 = FLAG_REG_CHANGE(sr) ;
        flag0 |= 1 ;
        if( value0 == 0 ) {
            flag0 |= 2 ; 
        }
    }
    else {
        value0 = se->reg[sr] ;
        flag0 = 0 ;
    }
    flag1 = 0 ;
    
    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, 100 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    // true test
    ENTL_DEBUG( "alo_rd_test (2) sr %04x dt %04x cond %d \n", sr, dt, cond ) ;

    de->reg[dt] = true_dt_values( se->reg[sr], cond) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    se->flags = 0 ;
    de->flags = 0 ;

    cc = test_values( se->reg[sr], de->reg[dt], cond ) ;
    // test as is 
    value1 = de->reg[dt] ;
    ENTL_DEBUG( "alo_rd_test (2) cond %d s %lx d %lx ->  %lx %lx\n", cond, se->reg[sr], de->reg[dt], value0, value1 ) ;
    if( cc ) {
        value0 = de->reg[dt] ;
        flag0 = FLAG_REG_CHANGE(sr) ;
        flag0 |= 1 ;
        if( value0 == 0 ) {
            flag0 |= 2 ; 
        }
    }
    else {
        value0 = se->reg[sr] ;
        flag0 = 0 ;
    }
    flag1 = 0 ;
    
    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, 100 ) ;

    if( !ret0 || !ret1 ) return 0 ;    


    if( cond == ALO_COND_ALLWAYS) return 1 ;

    // false test
    ENTL_DEBUG( "alo_rd_test (3) sr %04x dt %04x cond %d  \n", sr, dt, cond ) ;

    de->reg[dt] = false_dt_values( se->reg[sr], cond) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    se->flags = 0 ;
    de->flags = 0 ;

    cc = test_values( se->reg[sr], de->reg[dt], cond ) ;
    // test as is 
    value1 = de->reg[dt] ;
    ENTL_DEBUG( "alo_rd_test (3) cond %d s %lx d %lx ->  %lx %lx\n", cond, se->reg[sr], de->reg[dt], value0, value1 ) ;
    if( cc ) {
        value0 = de->reg[dt] ;
        flag0 = FLAG_REG_CHANGE(sr) ;
        flag0 |= 1 ;
        if( value0 == 0 ) {
            flag0 |= 2 ; 
        }
    }
    else {
        value0 = se->reg[sr] ;
        flag0 = 0 ;
    }
    flag1 = 0 ;
    
    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, 100 ) ;

    if( !ret0 || !ret1 ) return 0 ;    

    return 1 ;
}

int alo_wr_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt, uint16_t cond ) 
{
    int ret0, ret1 ;
    uint64_t value0, value1 ;
    uint16_t opcode = ALO_WR | cond ;
    uint32_t flag0, flag1 ;
    int cc ;

    //debug_flag = 1 ;

    ENTL_DEBUG( "alo_wr_test (1) sr %04x dt %04x cond %d  \n", sr, dt, cond ) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    se->flags = 0 ;
    de->flags = 0 ;

    cc = test_values( se->reg[sr], de->reg[dt], cond) ;
     
    value0 = se->reg[sr] ;
    flag0 = 0 ;
    if( cc ) {
        value1 = se->reg[sr] ;
        flag1 = FLAG_REG_CHANGE(dt) ;
        flag1 |= 1 ;
        if( value1 == 0 ) {
            flag1 |= 2 ; 
        }
    }
    else {
        value1 = de->reg[dt] ;
        flag1 = 0 ;
    }
    ENTL_DEBUG( "alo_wr_test (1) cond %d s %lx d %lx ->  %lx %lx\n", cond, se->reg[sr], de->reg[dt], value0, value1 ) ;
    
    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, 100 ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;

    // true test
    ENTL_DEBUG( "alo_wr_test (2) sr %04x dt %04x cond %d  \n", sr, dt, cond ) ;

    de->reg[dt] = true_dt_values( se->reg[sr], cond) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    se->flags = 0 ;
    de->flags = 0 ;

    cc = test_values( se->reg[sr], de->reg[dt], cond ) ;
    value0 = se->reg[sr] ;
    flag0 = 0 ;
    if( cc ) {
        value1 = se->reg[sr] ;
        flag1 = FLAG_REG_CHANGE(dt) ;
        flag1 |= 1 ;
        if( value1 == 0 ) {
            flag1 |= 2 ; 
        }
    }
    else {
        value1 = de->reg[dt] ;
        flag1 = 0 ;
    }
    ENTL_DEBUG( "alo_wr_test (2) cond %d s %lx d %lx ->  %lx %lx\n", cond, se->reg[sr], de->reg[dt], value0, value1 ) ;

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, 100 ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;    


    if( cond == ALO_COND_ALLWAYS) return 1 ;

    // false test
    ENTL_DEBUG( "alo_wr_test (3) sr %04x dt %04x cond %d  \n", sr, dt, cond ) ;

    de->reg[dt] = false_dt_values( se->reg[sr], cond) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    se->flags = 0 ;
    de->flags = 0 ;

    cc = test_values( se->reg[sr], de->reg[dt], cond ) ;
    value0 = se->reg[sr] ;
    flag0 = 0 ;
    if( cc ) {
        value1 = se->reg[sr] ;
        flag1 = FLAG_REG_CHANGE(dt) ;
        flag1 |= 1 ;
        if( value1 == 0 ) {
            flag1 |= 2 ; 
        }
    }
    else {
        value1 = de->reg[dt] ;
        flag1 = 0 ;
    }
    ENTL_DEBUG( "alo_wr_test (3) cond %d s %lx d %lx ->  %lx %lx\n", cond, se->reg[sr], de->reg[dt], value0, value1 ) ;
    
    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, 100 ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;    

    return 1 ;
}

int alo_swap_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt, uint16_t cond ) 
{
    int ret0, ret1 ;
    uint64_t value0, value1 ;
    uint16_t opcode = ALO_SWAP | cond ;
    uint32_t flag0, flag1 ;
    int cc ;

    //debug_flag = 1 ;

    ENTL_DEBUG( "alo_swap_test (1) sr %04x dt %04x cond %d  \n", sr, dt, cond ) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    se->flags = 0 ;
    de->flags = 0 ;

    cc = test_values( se->reg[sr], de->reg[dt], cond) ;
     

    if( cc ) {
        value0 = de->reg[dt] ;
        flag0 = FLAG_REG_CHANGE(sr) ;
        value1 = se->reg[sr] ;
        flag0 |= 1 ;
        if( value0 == 0 ) {
            flag0 |= 2 ; 
        }
        flag1 = FLAG_REG_CHANGE(dt) ;
        flag1 |= 1 ;
        if( value1 == 0 ) {
            flag1 |= 2 ; 
        }
    }
    else {
        value1 = de->reg[dt] ;
        flag1 = 0 ;
        value0 = se->reg[sr] ;
        flag0 = 0 ;
    }
    ENTL_DEBUG( "alo_swap_test (1) cond %d s %lx d %lx ->  %lx %lx\n", cond, se->reg[sr], de->reg[dt], value0, value1 ) ;
    
    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;

    // true test
    ENTL_DEBUG( "alo_swap_test (2) sr %04x dt %04x cond %d \n", sr, dt, cond ) ;

    de->reg[dt] = true_dt_values( se->reg[sr], cond) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    se->flags = 0 ;
    de->flags = 0 ;

    cc = test_values( se->reg[sr], de->reg[dt], cond ) ;
    if( cc ) {
        value0 = de->reg[dt] ;
        flag0 = FLAG_REG_CHANGE(sr) ;
        value1 = se->reg[sr] ;
        flag0 |= 1 ;
        if( value0 == 0 ) {
            flag0 |= 2 ; 
        }
        flag1 = FLAG_REG_CHANGE(dt) ;
        flag1 |= 1 ;
        if( value1 == 0 ) {
            flag1 |= 2 ; 
        }
    }
    else {
        value1 = de->reg[dt] ;
        flag1 = 0 ;
        value0 = se->reg[sr] ;
        flag0 = 0 ;
    }
    ENTL_DEBUG( "alo_swap_test (2) cond %d s %lx d %lx ->  %lx %lx\n", cond, se->reg[sr], de->reg[dt], value0, value1 ) ;

    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;    


    if( cond == ALO_COND_ALLWAYS) return 1 ;

    // false test
    ENTL_DEBUG( "alo_swap_test (3) sr %04x dt %04x cond %d \n", sr, dt, cond ) ;

    de->reg[dt] = false_dt_values( se->reg[sr], cond) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    se->flags = 0 ;
    de->flags = 0 ;

    cc = test_values( se->reg[sr], de->reg[dt], cond ) ;
    if( cc ) {
        value0 = de->reg[dt] ;
        flag0 = FLAG_REG_CHANGE(sr) ;
        value1 = se->reg[sr] ;
        flag0 |= 1 ;
        if( value0 == 0 ) {
            flag0 |= 2 ; 
        }
        flag1 = FLAG_REG_CHANGE(dt) ;
        flag1 |= 1 ;
        if( value1 == 0 ) {
            flag1 |= 2 ; 
        }
    }
    else {
        value1 = de->reg[dt] ;
        flag1 = 0 ;
        value0 = se->reg[sr] ;
        flag0 = 0 ;
    }
    ENTL_DEBUG( "alo_swap_test (3) cond %d s %lx d %lx ->  %lx %lx\n", cond, se->reg[sr], de->reg[dt], value0, value1 ) ;
    
    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;    

    return 1 ;
}

int alo_bset_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt, uint16_t bit ) 
{
    int ret0, ret1 ;
    uint64_t value0, value1 ;
    uint16_t opcode = ALO_BSET | bit ;
    uint32_t flag0, flag1 ;
    int cc ;

    //debug_flag = 1 ;

    ENTL_DEBUG( "alo_bset_test (1) sr %04x dt %04x bit %d %lx\n", sr, dt, bit , (uint64_t)1<<bit ) ;
    if( de->reg[dt] & ((uint64_t)1 << bit) ) de->reg[dt] = de->reg[dt] ^ ((uint64_t)1 << bit) ;
    if( se->reg[sr] & ((uint64_t)1 << bit) ) se->reg[sr] = se->reg[sr] ^ ((uint64_t)1 << bit) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    value1 = de->reg[dt] | ((uint64_t)1 << bit) ;
    flag1 = FLAG_REG_CHANGE(dt) | 1 ;
    value0 = se->reg[sr] | ((uint64_t)1 << bit) ;
    flag0 = FLAG_REG_CHANGE(sr) | 1 ;    

    ENTL_DEBUG( "alo_bset_test (1) bit %d s %lx d %lx ->  %lx %lx\n", bit, se->reg[sr], de->reg[dt], value0, value1 ) ;
    
    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;    


    return 1 ;

}

int alo_breset_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt, uint16_t bit ) 
{
    int ret0, ret1 ;
    uint64_t value0, value1 ;
    uint16_t opcode = ALO_BRESET | bit ;
    uint32_t flag0, flag1 ;
    int cc ;

    //debug_flag = 1 ;

    ENTL_DEBUG( "alo_breset_test (1) sr %04x dt %04x bit %d %lx\n", sr, dt, bit , (uint64_t)1<<bit) ;
    de->reg[dt] = de->reg[dt] | ((uint64_t)1 << bit) ;
    se->reg[sr] = se->reg[sr] | ((uint64_t)1 << bit) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    value1 = de->reg[dt] ^ ((uint64_t)1 << bit)  ;
    flag1 = FLAG_REG_CHANGE(dt) | 1 ;
    value0 = se->reg[sr] ^ ((uint64_t)1 << bit)  ;
    flag0 = FLAG_REG_CHANGE(sr) | 1 ;    

    ENTL_DEBUG( "alo_breset_test (1) bit %d s %lx d %lx ->  %lx %lx\n", bit, se->reg[sr], de->reg[dt], value0, value1 ) ;
    
    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;    


    return 1 ;

}

int alo_btestset_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt, uint16_t bit ) 
{
    int ret0, ret1 ;
    uint64_t value0, value1 ;
    uint16_t opcode = ALO_BTESTSET | bit ;
    uint32_t flag0, flag1 ;
    int cc ;

    //debug_flag = 1 ;

    // true test
    ENTL_DEBUG( "alo_btestset_test (1) sr %04x dt %04x bit %d %lx\n", sr, dt, bit , (uint64_t)1<<bit ) ;
    if( de->reg[dt] & ((uint64_t)1 << bit) ) de->reg[dt] = de->reg[dt] ^ ((uint64_t)1 << bit) ;
    if( se->reg[sr] & ((uint64_t)1 << bit) ) se->reg[sr] = se->reg[sr] ^ ((uint64_t)1 << bit) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    value1 = de->reg[dt] | ((uint64_t)1 << bit) ;
    flag1 = FLAG_REG_CHANGE(dt) | 1 ;
    value0 = se->reg[sr] | ((uint64_t)1 << bit) ;
    flag0 = FLAG_REG_CHANGE(sr) | 1 ;    

    ENTL_DEBUG( "alo_btestset_test (1) bit %d s %lx d %lx ->  %lx %lx\n", bit, se->reg[sr], de->reg[dt], value0, value1 ) ;
    
    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;    

    // fail on source
    ENTL_DEBUG( "alo_btestset_test (2) sr %04x dt %04x bit %d \n", sr, dt, bit ) ;
    if( de->reg[dt] & ((uint64_t)1 << bit) ) de->reg[dt] = de->reg[dt] ^ ((uint64_t)1 << bit) ;
    //if( se->reg[sr] & ((uint64_t)1 << bit) ) se->reg[sr] = se->reg[sr] ^ (1 << bit) ;
    se->flags = 0 ;
    de->flags = 0 ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    value1 = de->reg[dt] ;
    flag1 = 0 ;
    value0 = se->reg[sr] ;
    flag0 = 0 ;    

    ENTL_DEBUG( "alo_btestset_test (2) bit %d s %lx d %lx ->  %lx %lx\n", bit, se->reg[sr], de->reg[dt], value0, value1 ) ;
    
    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;        

    // fail on destination
    ENTL_DEBUG( "alo_btestset_test (3) sr %04x dt %04x bit %d \n", sr, dt, bit ) ;
    if( se->reg[sr] & ((uint64_t)1 << bit) ) se->reg[sr] = se->reg[sr] ^ ((uint64_t)1 << bit) ;
    de->reg[dt] = de->reg[dt] | ((uint64_t)1 << bit) ;
    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    value1 = de->reg[dt] ;
    flag1 = 0 ;
    value0 = se->reg[sr] ;
    flag0 = 0 ;    

    ENTL_DEBUG( "alo_btestset_test (3) bit %d s %lx d %lx ->  %lx %lx\n", bit, se->reg[sr], de->reg[dt], value0, value1 ) ;
    
    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;    

    return 1 ;

}

int alo_btestreset_test( alo_regs_t *se, alo_regs_t *de, uint16_t sr, uint16_t dt, uint16_t bit ) 
{
    int ret0, ret1 ;
    uint64_t value0, value1 ;
    uint16_t opcode = ALO_BTESTRESET | bit ;
    uint32_t flag0, flag1 ;
    int cc ;

    //debug_flag = 1 ;

    // true test
    ENTL_DEBUG( "alo_btestset_test (1) sr %04x dt %04x bit %d %lx\n", sr, dt, bit , (uint64_t)1<<bit ) ;
    de->reg[dt] = de->reg[dt] | ((uint64_t)1 << bit) ;
    se->reg[sr] = se->reg[sr] | ((uint64_t)1 << bit) ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    value1 = de->reg[dt] ^ ((uint64_t)1 << bit) ;
    flag1 = FLAG_REG_CHANGE(dt) | 1 ;
    value0 = se->reg[sr] ^ ((uint64_t)1 << bit) ;
    flag0 = FLAG_REG_CHANGE(sr) | 1 ;    

    ENTL_DEBUG( "alo_btestset_test (1) bit %d s %lx d %lx ->  %lx %lx\n", bit, se->reg[sr], de->reg[dt], value0, value1 ) ;
    
    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;    

    // fail on source
    ENTL_DEBUG( "alo_btestset_test (2) sr %04x dt %04x bit %d \n", sr, dt, bit ) ;
    de->reg[dt] = de->reg[dt] | ((uint64_t)1 << bit) ;
    //se->reg[sr] = se->reg[sr] ^ (1 << bit) ;
    se->flags = 0 ;
    de->flags = 0 ;

    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    value1 = de->reg[dt] ;
    flag1 = 0 ;
    value0 = se->reg[sr] ;
    flag0 = 0 ;    

    ENTL_DEBUG( "alo_btestset_test (2) bit %d s %lx d %lx ->  %lx %lx\n", bit, se->reg[sr], de->reg[dt], value0, value1 ) ;
    
    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;        

    // fail on destination
    ENTL_DEBUG( "alo_btestset_test (3) sr %04x dt %04x bit %d \n", sr, dt, bit ) ;
    se->reg[sr] = se->reg[sr] | ((uint64_t)1 << bit) ;
    de->reg[dt] = de->reg[dt] ^ ((uint64_t)1 << bit) ;
    copy_regs( se, &src_copy ) ;
    copy_regs( de, &dst_copy ) ;

    value1 = de->reg[dt] ;
    flag1 = 0 ;
    value0 = se->reg[sr] ;
    flag0 = 0 ;    

    ENTL_DEBUG( "alo_btestset_test (3) bit %d s %lx d %lx ->  %lx %lx\n", bit, se->reg[sr], de->reg[dt], value0, value1 ) ;
    
    alo_exec_ops( se, de, sr, dt, opcode ) ;

    ret0 = alo_check_result( "se1", se, sr, value0 , flag0 ) ;
    ret1 = alo_check_result( "de1", de, dt, value1 , flag1 ) ;

    if( !ret0 || !ret1 ) return 0 ;

    ret0 = check_untouched( se, &src_copy, sr ) ;  
    ret1 = check_untouched( de, &dst_copy, dt ) ;

    if( !ret0 || !ret1 ) return 0 ;    

    return 1 ;

}

