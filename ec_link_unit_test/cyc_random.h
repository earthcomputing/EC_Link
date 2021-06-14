/*---------------------------------------------------------------------------------------------
 *  Copyright © 2017-present Earth Computing Corporation. All rights reserved.
 *  Licensed under the MIT License. See LICENSE.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef _CYC_RANDOM_H_
#define _CYC_RANDOM_H_

#ifdef __cplusplus 
extern "C" {
#endif

typedef struct JD_mt_random_s mt_random_t ;

/* #define N 624 */

struct JD_mt_random_s {
  unsigned long mt[624] ; /* the array for the state vector  */
  int mti ; /* mti==N+1 means mt[N] is not initialized */
} ;

// return random if int_value == 0, else initialize with int_value
int mt_irandom( int int_value ) ;

unsigned int mt_urandom( int int_value ) ; // return unsigned rand value if int_value == 0, else init with int_value ;


#define DEFAULT_PDF_SIZE 64

typedef struct cyc_random_s cyc_random_t ;

struct cyc_random_s {
  int seed ;
  int width ;
  int act_width ;
  int next_zero ;
  int swap_point ;
  uint64_t rnd_data ;
  uint64_t data ;
  uint64_t sdata ;
  uint64_t zdata ;
  uint64_t pdata ;
  uint64_t mdata ;
  uint64_t ppdata ;
  uint64_t udata ;
  mt_random_t  *rnd ;
  
} ;

/* allocate cyc_random */
cyc_random_t *alloc_cyc_random( int width ) ;
uint64_t cyc_random( cyc_random_t *rd ) ;

#ifdef __cplusplus 
}
#endif

#endif 
