/*
 * Copyright (C) 2017,2018  Earth Computing Inc.  All rights reserved.
 *
 *
 * @file          alo_test_main.cpp
 * @brief         Atomic Link Operation Unit Test main
 *
 * Author:        Atsushi Kasuya
 *
 */

#include <iostream>
#include <time.h>
#include <string>
#include <unistd.h>
#include "alo_tester.h"
#include "cyc_random.h"
#include "gtest/gtest.h"


static alo_regs_t se ;
static alo_regs_t de ;
static cyc_random_t *rd0, *rd1, *rbit ;

class AloUnitTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        clear_regs( &se ) ;
        clear_regs( &de ) ;  
        rd0 = alloc_cyc_random( 5 ) ;
        rd1 = alloc_cyc_random( 5 ) ;
        rbit = alloc_cyc_random( 6 ) ;
    }
    
    virtual void TearDown() {
        printf( "AloUnitTest %s done\n", testName.c_str() ) ;
    }
    
    std::string testName ;
};


// Testing NOP instruction
TEST_F( AloUnitTest, AloNopTest ) {
    int i, result, sr, dt ;

    for( i = 0 ; i < 32 ; i++ ) {
        rand_regs( &se ) ;
        rand_regs( &de ) ;
        sr = cyc_random(rd0) ;
        dt = cyc_random(rd1) ;
        result = alo_nop_test( &se, &de, sr, dt) ;

        EXPECT_EQ( result, 1 ) ;
    }

}

// Testing ADD instruction
TEST_F( AloUnitTest, AloAddTest ) {
    int i, result, sr, dt ;

    for( i = 0 ; i < 32 ; i++ ) {
        rand_regs( &se ) ;
        rand_regs( &de ) ;
        sr = cyc_random(rd0) ;
        dt = cyc_random(rd1) ;
        //printf( "AloAddTest %d sr %d dt %d\n", i, sr, dt) ;
        result = alo_add_test( &se, &de, sr, dt) ;

        EXPECT_EQ( result, 1 ) ;
    }

}


// Testing SUBS instruction
TEST_F( AloUnitTest, AloSubsTest ) {
    int i, result, sr, dt ;

    for( i = 0 ; i < 32 ; i++ ) {
        rand_regs( &se ) ;
        rand_regs( &de ) ;
        sr = cyc_random(rd0) ;
        dt = cyc_random(rd1) ;
        //printf( "AloAddTest %d sr %d dt %d\n", i, sr, dt) ;
        result = alo_subs_test( &se, &de, sr, dt) ;

        EXPECT_EQ( result, 1 ) ;
    }

}

// Testing SUBD instruction
TEST_F( AloUnitTest, AloSubdTest ) {
    int i, result, sr, dt ;

    for( i = 0 ; i < 32 ; i++ ) {
        rand_regs( &se ) ;
        rand_regs( &de ) ;
        sr = cyc_random(rd0) ;
        dt = cyc_random(rd1) ;
        //printf( "AloAddTest %d sr %d dt %d\n", i, sr, dt) ;
        result = alo_subd_test( &se, &de, sr, dt) ;

        EXPECT_EQ( result, 1 ) ;
    }

}

// Testing SUBD instruction
TEST_F( AloUnitTest, AloAndTest ) {
    int i, result, sr, dt ;

    for( i = 0 ; i < 32 ; i++ ) {
        rand_regs( &se ) ;
        rand_regs( &de ) ;
        sr = cyc_random(rd0) ;
        dt = cyc_random(rd1) ;
        //printf( "AloAddTest %d sr %d dt %d\n", i, sr, dt) ;
        result = alo_and_test( &se, &de, sr, dt) ;

        EXPECT_EQ( result, 1 ) ;
    }

}

// Testing OR instruction
TEST_F( AloUnitTest, AloOrTest ) {
    int i, result, sr, dt ;

    for( i = 0 ; i < 32 ; i++ ) {
        rand_regs( &se ) ;
        rand_regs( &de ) ;
        sr = cyc_random(rd0) ;
        dt = cyc_random(rd1) ;
        //printf( "AloAddTest %d sr %d dt %d\n", i, sr, dt) ;
        result = alo_or_test( &se, &de, sr, dt) ;

        EXPECT_EQ( result, 1 ) ;
    }

}

// Testing INC instruction
TEST_F( AloUnitTest, AloIncTest ) {
    int i, result, sr, dt ;

    for( i = 0 ; i < 32 ; i++ ) {
        rand_regs( &se ) ;
        rand_regs( &de ) ;
        sr = cyc_random(rd0) ;
        dt = cyc_random(rd1) ;
        //printf( "AloAddTest %d sr %d dt %d\n", i, sr, dt) ;
        result = alo_inc_test( &se, &de, sr, dt) ;

        EXPECT_EQ( result, 1 ) ;
    }

}



// Testing DEC instruction
TEST_F( AloUnitTest, AloDecTest ) {
    int i, result, sr, dt ;

    for( i = 0 ; i < 32 ; i++ ) {
        rand_regs( &se ) ;
        rand_regs( &de ) ;
        sr = cyc_random(rd0) ;
        dt = cyc_random(rd1) ;
        //printf( "AloAddTest %d sr %d dt %d\n", i, sr, dt) ;
        result = alo_dec_test( &se, &de, sr, dt) ;

        EXPECT_EQ( result, 1 ) ;
    }

}


// Testing INC instruction
TEST_F( AloUnitTest, AloIncsTest ) {
    int i, result, sr, dt ;

    for( i = 0 ; i < 32 ; i++ ) {
        rand_regs( &se ) ;
        rand_regs( &de ) ;
        sr = cyc_random(rd0) ;
        dt = cyc_random(rd1) ;
        //printf( "AloAddTest %d sr %d dt %d\n", i, sr, dt) ;
        result = alo_incs_test( &se, &de, sr, dt) ;

        EXPECT_EQ( result, 1 ) ;
    }

}

// Testing DEC instruction
TEST_F( AloUnitTest, AloDecsTest ) {
    int i, result, sr, dt ;

    for( i = 0 ; i < 32 ; i++ ) {
        rand_regs( &se ) ;
        rand_regs( &de ) ;
        sr = cyc_random(rd0) ;
        dt = cyc_random(rd1) ;
        //printf( "AloAddTest %d sr %d dt %d\n", i, sr, dt) ;
        result = alo_decs_test( &se, &de, sr, dt) ;

        EXPECT_EQ( result, 1 ) ;
    }

}


// Testing INC instruction
TEST_F( AloUnitTest, AloIncdTest ) {
    int i, result, sr, dt ;

    for( i = 0 ; i < 32 ; i++ ) {
        rand_regs( &se ) ;
        rand_regs( &de ) ;
        sr = cyc_random(rd0) ;
        dt = cyc_random(rd1) ;
        //printf( "AloAddTest %d sr %d dt %d\n", i, sr, dt) ;
        result = alo_incd_test( &se, &de, sr, dt) ;

        EXPECT_EQ( result, 1 ) ;
    }

}

// Testing DEC instruction
TEST_F( AloUnitTest, AloDecdTest ) {
    int i, result, sr, dt ;

    for( i = 0 ; i < 32 ; i++ ) {
        rand_regs( &se ) ;
        rand_regs( &de ) ;
        sr = cyc_random(rd0) ;
        dt = cyc_random(rd1) ;
        //printf( "AloAddTest %d sr %d dt %d\n", i, sr, dt) ;
        result = alo_decd_test( &se, &de, sr, dt) ;

        EXPECT_EQ( result, 1 ) ;
    }

}

// Testing RD instruction
TEST_F( AloUnitTest, AloRdTest ) {
    int i, result, sr, dt ;

    for( i = 0 ; i < 32 ; i++ ) {
        rand_regs( &se ) ;
        rand_regs( &de ) ;
        sr = cyc_random(rd0) ;
        dt = cyc_random(rd1) ;
        //printf( "AloAddTest %d sr %d dt %d\n", i, sr, dt) ;
        result = alo_rd_test( &se, &de, sr, dt, ALO_COND_ALLWAYS) ;

        EXPECT_EQ( result, 1 ) ;
        rand_regs( &se ) ;
        rand_regs( &de ) ;
        result = alo_rd_test( &se, &de, sr, dt, ALO_COND_EQ) ;
        EXPECT_EQ( result, 1 ) ;
 
        rand_regs( &se ) ;
        rand_regs( &de ) ;
        result = alo_rd_test( &se, &de, sr, dt, ALO_COND_LT) ;
        EXPECT_EQ( result, 1 ) ;

        rand_regs( &se ) ;
        rand_regs( &de ) ;
        result = alo_rd_test( &se, &de, sr, dt, ALO_COND_LE) ;
        EXPECT_EQ( result, 1 ) ;

        rand_regs( &se ) ;
        rand_regs( &de ) ;
        result = alo_rd_test( &se, &de, sr, dt, ALO_COND_GT) ;
        EXPECT_EQ( result, 1 ) ;

        rand_regs( &se ) ;
        rand_regs( &de ) ;
        result = alo_rd_test( &se, &de, sr, dt, ALO_COND_GE) ;
        EXPECT_EQ( result, 1 ) ;
               
    }

}

// Testing WR instruction
TEST_F( AloUnitTest, AloWrTest ) {
    int i, result, sr, dt ;

    for( i = 0 ; i < 32 ; i++ ) {
        rand_regs( &se ) ;
        rand_regs( &de ) ;
        sr = cyc_random(rd0) ;
        dt = cyc_random(rd1) ;
        //printf( "AloAddTest %d sr %d dt %d\n", i, sr, dt) ;
        result = alo_wr_test( &se, &de, sr, dt, ALO_COND_ALLWAYS) ;

        EXPECT_EQ( result, 1 ) ;
        rand_regs( &se ) ;
        rand_regs( &de ) ;
        result = alo_wr_test( &se, &de, sr, dt, ALO_COND_EQ) ;
        EXPECT_EQ( result, 1 ) ;
 
        rand_regs( &se ) ;
        rand_regs( &de ) ;
        result = alo_wr_test( &se, &de, sr, dt, ALO_COND_LT) ;
        EXPECT_EQ( result, 1 ) ;

        rand_regs( &se ) ;
        rand_regs( &de ) ;
        result = alo_wr_test( &se, &de, sr, dt, ALO_COND_LE) ;
        EXPECT_EQ( result, 1 ) ;

        rand_regs( &se ) ;
        rand_regs( &de ) ;
        result = alo_wr_test( &se, &de, sr, dt, ALO_COND_GT) ;
        EXPECT_EQ( result, 1 ) ;

        rand_regs( &se ) ;
        rand_regs( &de ) ;
        result = alo_wr_test( &se, &de, sr, dt, ALO_COND_GE) ;
        EXPECT_EQ( result, 1 ) ;
               
    }

}

// Testing SWAP instruction
TEST_F( AloUnitTest, AloSwapTest ) {
    int i, result, sr, dt ;

    for( i = 0 ; i < 32 ; i++ ) {
        rand_regs( &se ) ;
        rand_regs( &de ) ;
        sr = cyc_random(rd0) ;
        dt = cyc_random(rd1) ;
        //printf( "AloAddTest %d sr %d dt %d\n", i, sr, dt) ;
        result = alo_swap_test( &se, &de, sr, dt, ALO_COND_ALLWAYS) ;

        EXPECT_EQ( result, 1 ) ;
        rand_regs( &se ) ;
        rand_regs( &de ) ;
        result = alo_swap_test( &se, &de, sr, dt, ALO_COND_EQ) ;
        EXPECT_EQ( result, 1 ) ;
 
        rand_regs( &se ) ;
        rand_regs( &de ) ;
        result = alo_swap_test( &se, &de, sr, dt, ALO_COND_LT) ;
        EXPECT_EQ( result, 1 ) ;

        rand_regs( &se ) ;
        rand_regs( &de ) ;
        result = alo_swap_test( &se, &de, sr, dt, ALO_COND_LE) ;
        EXPECT_EQ( result, 1 ) ;

        rand_regs( &se ) ;
        rand_regs( &de ) ;
        result = alo_swap_test( &se, &de, sr, dt, ALO_COND_GT) ;
        EXPECT_EQ( result, 1 ) ;

        rand_regs( &se ) ;
        rand_regs( &de ) ;
        result = alo_swap_test( &se, &de, sr, dt, ALO_COND_GE) ;
        EXPECT_EQ( result, 1 ) ;
               
    }

}

// Testing BSET instruction
TEST_F( AloUnitTest, AloBsetTest ) {
    int i, j, result, sr, dt, bit ;

    for( i = 0 ; i < 32 ; i++ ) {
        sr = cyc_random(rd0) ;
        dt = cyc_random(rd1) ;
        for( j = 0 ; j < 64 ; j++) {
            rand_regs( &se ) ;
            rand_regs( &de ) ;
            bit = cyc_random(rbit) ;
            result = alo_bset_test( &se, &de, sr, dt, bit) ;
            EXPECT_EQ( result, 1 ) ;
        }
        
    }

}


// Testing BRESET instruction
TEST_F( AloUnitTest, AloBresetTest ) {
    int i, j, result, sr, dt, bit ;

    for( i = 0 ; i < 32 ; i++ ) {
        sr = cyc_random(rd0) ;
        dt = cyc_random(rd1) ;
        for( j = 0 ; j < 64 ; j++) {
            rand_regs( &se ) ;
            rand_regs( &de ) ;
            bit = cyc_random(rbit) ;
            result = alo_breset_test( &se, &de, sr, dt, bit) ;
            EXPECT_EQ( result, 1 ) ;
        }
        
    }

}

// Testing BTESTSET instruction
TEST_F( AloUnitTest, AloBtestsetTest ) {
    int i, j, result, sr, dt, bit ;

    for( i = 0 ; i < 32 ; i++ ) {
        sr = cyc_random(rd0) ;
        dt = cyc_random(rd1) ;
        for( j = 0 ; j < 64 ; j++) {
            rand_regs( &se ) ;
            rand_regs( &de ) ;
            bit = cyc_random(rbit) ;
            result = alo_btestset_test( &se, &de, sr, dt, bit) ;
            EXPECT_EQ( result, 1 ) ;
        }
        
    }

}

// Testing BTESTRESET instruction
TEST_F( AloUnitTest, AloBtestresetTest ) {
    int i, j, result, sr, dt, bit ;

    for( i = 0 ; i < 32 ; i++ ) {
        sr = cyc_random(rd0) ;
        dt = cyc_random(rd1) ;
        for( j = 0 ; j < 64 ; j++) {
            rand_regs( &se ) ;
            rand_regs( &de ) ;
            bit = cyc_random(rbit) ;
            result = alo_btestreset_test( &se, &de, sr, dt, bit) ;
            EXPECT_EQ( result, 1 ) ;
        }
        
    }

}



GTEST_API_ int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);


    return RUN_ALL_TESTS();
}
