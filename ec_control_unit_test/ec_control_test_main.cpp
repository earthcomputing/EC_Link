/*
 * Copyright (C) 2018,  Earth Computing Inc.  All rights reserved.
 *
 *
 * @file          ec_control_test_main.cpp
 * @brief         ENTL State Machine Unit Test main
 *
 * Author:        Atsushi Kasuya
 *
 */

#include <iostream>
#include <time.h>
#include <string>
#include <unistd.h>
#include "ec_control_tester.h"
#include "cyc_random.h"
#include "gtest/gtest.h"

static entl_state_machine_t mcn0, mcn1 ;

class EC_ControlUnitTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        //ec_link_init( &reg ) ;
        //rd0 = alloc_cyc_random( 5 ) ;
        //rd1 = alloc_cyc_random( 5 ) ;
        //rbit = alloc_cyc_random( 6 ) ;
    }
    
    virtual void TearDown() {
        printf( "EC_ControlUnitTest %s done\n", testName.c_str() ) ;
    }
    
    std::string testName ;
};


// Testing simple healthy loop
TEST_F( EC_ControlUnitTest, EC_ControlHealthyLoopUnitTest ) {
    int result ;

    result = healthy_loop( &mcn0, &mcn1 ) ;

    EXPECT_EQ( result, 1 ) ;

}

// Testing EC hiccup case
TEST_F( EC_ControlUnitTest, EC_ControlTT_HiccupUnitTest ) {
    int result ;

    result = tt_hiccup( &mcn0, &mcn1 ) ;

    EXPECT_EQ( result, 1 ) ;

}


// Testing EC error resend case
TEST_F( EC_ControlUnitTest, EC_ControlTC_ResendUnitTest ) {
    int result ;

    result = tc_resend( &mcn0, &mcn1 ) ;

    EXPECT_EQ( result, 1 ) ;

}


// Testing AIT sequence case
TEST_F( EC_ControlUnitTest, EC_ControlAIT_SequenceUnitTest ) {
    int result ;

    result = entt_ait_sequence( &mcn0, &mcn1 ) ;

    EXPECT_EQ( result, 1 ) ;

}


// Testing ALO sequence case
TEST_F( EC_ControlUnitTest, EC_ControlALO_SequenceUnitTest ) {
    int result ;

    result = entt_alo_sequence( &mcn0, &mcn1 ) ;

    EXPECT_EQ( result, 1 ) ;

}
/*
// Testing EC ALO resend case
TEST_F( EC_ControlUnitTest, EC_ControlALO_ResendUnitTest ) {
    int result ;

    result = entt_alo_resend( &mcn0, &mcn1 ) ;

    EXPECT_EQ( result, 1 ) ;

}
*/

GTEST_API_ int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);


    return RUN_ALL_TESTS();
}
