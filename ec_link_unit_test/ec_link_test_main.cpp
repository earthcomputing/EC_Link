/*
 * Copyright (C) 2017,  Earth Computing Inc.  All rights reserved.
 *
 *
 * @file          ec_link_test_main.cpp
 * @brief         ENTL State Machine Unit Test main
 *
 * Author:        Atsushi Kasuya
 *
 */

#include <iostream>
#include <time.h>
#include <string>
#include <unistd.h>
#include "ec_link_tester.h"
#include "cyc_random.h"
#include "gtest/gtest.h"

static ec_link_reg_t reg ;

class EC_LinkUnitTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        ec_link_init( &reg ) ;
        //rd0 = alloc_cyc_random( 5 ) ;
        //rd1 = alloc_cyc_random( 5 ) ;
        //rbit = alloc_cyc_random( 6 ) ;
    }
    
    virtual void TearDown() {
        printf( "EC_LinkUnitTest %s done\n", testName.c_str() ) ;
    }
    
    std::string testName ;
};


// Testing simple healthy loop
TEST_F( EC_LinkUnitTest, EC_LinkHealthyLoopUnitTest ) {
    int result ;

    result = healthy_loop( &reg ) ;

    EXPECT_EQ( result, 1 ) ;

}

// Testing hiccupping but not yet resend
TEST_F( EC_LinkUnitTest, EC_LinkTT_HiccupUnitTest ) {
    int result ;

    result = tt_hiccup( &reg ) ;

    EXPECT_EQ( result, 1 ) ;

}

// Testing hiccopped enough for resending
TEST_F( EC_LinkUnitTest, EC_LinkTC_ResendUnitTest ) {
    int result ;

    result = tc_resend( &reg ) ;

    EXPECT_EQ( result, 1 ) ;

}

// Testing hiccopped enough for resending
TEST_F( EC_LinkUnitTest, EC_LinkENTT_LoopUnitTest ) {
    int result ;

    result = entt_loop( &reg ) ;

    EXPECT_EQ( result, 1 ) ;

}

GTEST_API_ int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);


    return RUN_ALL_TESTS();
}
