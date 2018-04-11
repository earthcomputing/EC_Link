#
# Copyright (C) 2018 Earth Computing Inc.  All rights reserved.
#
cmake_minimum_required(VERSION 3.3)
project(ec_link_unit_test)

set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)

#set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake)

#include(CodeCoverage)

SET(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
SET(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")

# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS} . .. )

#add_definitions( -DNETRONOME_HOST )

# Link runTests with what we want to test and the GTest and pthread library
add_executable(runTests ec_link_test_main.cpp ec_link_tester.c cyc_random.c ../ec_link.c )

#set(Coverage_NAME "coverage_target")
#message( "set :"${Coverage_NAME}) 
#setup_target_for_coverage(coverage runTests coverage_report)

target_link_libraries(runTests ${GTEST_LIBRARIES} pthread gcov)
#target_link_libraries(runTests ${GTEST_LIBRARIES} pthread)