/*
 * Copyright (C) 2018,  Earth Computing Inc.  All rights reserved.
 *
 *
 * @file          ec_control.h
 * @brief         ENTL state machine 
 *
 * Author:        Atsushi Kasuya
 *
 */

#ifndef _EC_CONTROL_H_
#define _EC_CONTROL_H_

#ifdef __cplusplus 
extern "C" {
#endif

#include "ec_link.h"


#define ETH_P_ECLP  0xEAC0    /* Earth Computing Link Protocol [ NOT AN OFFICIALLY REGISTERED ID ] */

// MAC Destination Field Extraction Masks
#define ECLP_FW_MASK        (uint64_t)0x800000000000
#define ECLP_ETYPE_MASK     (uint64_t)0x001f00000000
#define ECLP_OP_ST_MASK     (uint64_t)0x0000ffff0000
#define ECLP_VALUE_MASK     (uint64_t)0x00000000ffff

// MAC Source Field Extraction Masks
#define ECLP_D_MASK         (uint64_t)0x800000000000
#define ECLP_H_MASK         (uint64_t)0x400000000000
#define ECLP_CREDIT_MASK    (uint64_t)0x3fff00000000
#define ECLP_NLKUP_MASK     (uint64_t)0x0000ffffffff

#define GET_ECLP_ETYPE(x)     (x&ECLP_ETYPE_MASK)
#define GET_ECLP_VALUE(x)     (x&ECLP_VALUE_MASK)
#define GET_ECLP_DIRECTION(x) ((x&ECLP_D_MASK)>>47)
#define GET_ECLP_CREDIT(x)    ((x&ECLP_CREDIT_MASK)>>32)
#define GET_ECLP_NLKUP(x)     (x&ECLP_NLKUP_MASK)


#ifdef __cplusplus 
}
#endif

#endif /* _EC_CONTROL_H_ */
