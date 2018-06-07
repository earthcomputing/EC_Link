
/*
 * Copyright (C) 2017,  Earth Computing Inc.  All rights reserved.
 *
 *
 * @file          cyc_random.c
 * @brief         Cyclic random generator 
 *
 * Author:        Atsushi Kasuya
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "cyc_random.h"

/*********************************************************************
 *  Mersenne Twister random function (Of period of 2**19937-1)
 *   developped by Makoto Matsumoto and Takuji Nishimura.
 *  Refer to the websuite for futher information.
 *  http://www.math.keio.ac.jp/~matumoto/emt.html
 *********************************************************************/


#include "mt19937_port.h"

/***********************************************************************
 * Functions that support MT random 
 ***********************************************************************/

static mt_random_t *a_mt_random_data = NULL ;

int mt_irandom( int int_value )
{
  int seed ;
  
  if( a_mt_random_data == NULL ) {
    a_mt_random_data = alloc_mt_random() ;
    sgenrand(4357, a_mt_random_data); 
      /* any nonzero integer can be used as a seed */
  }
  
  if( int_value ) {

    seed = int_value ;
    /* sgenrand(seed, a_mt_random_data) ; */
    init_genrand(seed, a_mt_random_data) ;
    
  }
    
  return genrand(a_mt_random_data) & 0x7fffffff ;
}

unsigned int mt_urandom( int int_value )
{
  int seed ;
  
  if( a_mt_random_data == NULL ) {
    a_mt_random_data = alloc_mt_random() ;
    sgenrand(4357, a_mt_random_data); 
      /* any nonzero integer can be used as a seed */
  }
  
  if( int_value ) {

    seed = int_value ;
    /* sgenrand(seed, a_mt_random_data) ; */
    init_genrand(seed, a_mt_random_data) ;
    
  }
    
  return genrand(a_mt_random_data) ;
}

/*************************************************************
 * class sys_CYC                                             *
 *************************************************************/
#define B0 0x00000001
#define B1 0x00000002
#define B2 0x00000004
#define B3 0x00000008
#define B4 0x00000010
#define B5 0x00000020
#define B6 0x00000040
#define B7 0x00000080
#define B8 0x00000100

/* primitive polinimials modulo 2 */
static int primpoli[] =
{
   B0,                                            /*  1, 0 */
  (B1),                                     /*  2, 1, 0 */
  (B1),                                     /*  3, 1, 0 */
  (B1),                                     /*  4, 1, 0 */
  (B2),                                     /*  5, 2, 0 */
  (B1),                                     /*  6, 1, 0 */
  (B1),                                     /*  7, 1, 0 */
  (B4 | B3 | B2),                           /*  8, 4, 3, 2, 0 */
  (B4),                                     /*  9, 4, 0 */
  (B3),                                     /* 10, 3, 0 */
  (B2),                                     /* 11, 2, 0 */
  (B6 | B4 | B1),                           /* 12, 6, 4, 1, 0 */
  (B4 | B3 | B1),                           /* 13, 4, 3, 1, 0 */
  (B5 | B3 | B1),                           /* 14, 5, 3, 1, 0 */
  (B1),                                     /* 15, 1, 0 */
  (B5 | B3 | B2),                           /* 16, 5, 3, 2, 0 */
  (B3),                                     /* 17, 3, 0 */
  (B5 | B2 | B1),                           /* 18, 5, 2, 1, 0 */
  (B5 | B2 | B1),                           /* 19, 5, 2, 1, 0 */
  (B3),                                     /* 20, 3, 0 */
  (B2),                                     /* 21, 2, 0 */
  (B1),                                     /* 22, 1, 0 */
  (B5),                                     /* 23, 5, 0 */
  (B4 | B3 | B1),                           /* 24, 4, 3, 1, 0 */
  (B3),                                     /* 25, 3, 0 */
  (B6 | B2 | B1),                           /* 26, 6, 2, 1, 0 */
  (B5 | B2 | B1),                           /* 27, 5, 2, 1, 0 */
  (B3),                                     /* 28, 3, 0 */
  (B2),                                     /* 29, 2, 0 */
  (B6 | B4 | B1),                           /* 30, 6, 4, 1, 0 */
  (B3),                                     /* 31, 3, 0 */
  (B7 | B5 | B3 | B2 | B1),                 /* 32, 7, 5, 3, 2, 1, 0 */
  (B6 | B4 | B1),                           /* 33, 6, 4, 1, 0 */
  (B7 | B6 | B5 | B2 | B1),                 /* 34, 7, 6, 5, 2, 1, 0 */
  (B2),                                     /* 35, 2, 0 */
  (B6 | B5 | B4 | B2 | B1),                 /* 36, 6, 5, 4, 2, 1, 0 */
  (B5 | B4 | B3 | B2 | B1),                 /* 37, 5, 4, 3, 2, 1, 0 */
  (B5 | B4 | B1),                           /* 38, 5, 4, 1, 0 */
  (B4),                                     /* 39, 4, 0 */
  (B5 | B4 | B3),                           /* 40, 5, 4, 3, 0 */
  (B3),                                     /* 41, 3, 0 */
  (B5 | B4 | B3 | B2 | B1),                 /* 42, 5, 4, 3, 2, 1, 0 */
  (B6 | B4 | B3),                           /* 43, 6, 4, 3, 0 */
  (B6 | B5 | B2),                           /* 44, 6, 5, 2, 0 */
  (B4 | B3 | B1),                           /* 45, 4, 3, 1, 0 */
  (B8 | B5 | B3 | B2 | B1),                 /* 46, 8, 5, 3, 2, 1, 0 */
  (B5),                                     /* 47, 5, 0 */
  (B7 | B5 | B4 | B2 | B1),                 /* 48, 7, 5, 4, 2, 1, 0 */
  (B6 | B5 | B4),                           /* 49, 6, 5, 4, 0 */
  (B4 | B3 | B2),                           /* 50, 4, 3, 2, 0 */
  (B6 | B3 | B1),                           /* 51, 6, 3, 1, 0 */
  (B3),                                     /* 52, 3, 0 */
  (B6 | B2 | B1),                           /* 53, 6, 2, 1, 0 */
  (B6 | B5 | B4 | B3 | B2),                 /* 54, 6, 5, 4, 3, 2, 0 */
  (B6 | B2 | B1),                           /* 55, 6, 2, 1, 0 */
  (B7 | B4 | B2),                           /* 56, 7, 4, 2, 0 */
  (B5 | B3 | B2),                           /* 57, 5, 3, 2, 0 */
  (B6 | B5 | B1),                           /* 58, 6, 5, 1, 0 */
  (B6 | B5 | B4 | B3 | B1),                 /* 59, 6, 5, 4, 3, 1, 0 */
  (B1),                                     /* 60, 1, 0 */
  (B5 | B2 | B1),                           /* 61, 5, 2, 1, 0 */
  (B6 | B5 | B3),                           /* 62, 6, 5, 3, 0 */
  (B1),                                     /* 63, 1, 0 */
  (B4 | B3 | B1),                           /* 64, 4, 3, 1, 0 */
  (B4 | B3 | B1),                           /* 65, 4, 3, 1, 0 */
  (B8 | B6 | B5 | B3 | B2),                 /* 66, 8, 6, 5, 3, 2, 0 */
  (B5 | B2 | B1),                           /* 67, 5, 2, 1, 0 */
  (B7 | B5 | B1),                           /* 68, 7, 5, 1, 0 */
  (B6 | B5 | B2),                           /* 69, 6, 5, 2, 0 */
  (B5 | B3 | B1),                           /* 70, 5, 3, 1, 0 */
  (B5 | B3 | B1),                           /* 71, 5, 3, 1, 0 */
  (B6 | B4 | B3 | B2 | B1),                 /* 72, 6, 4, 3, 2, 1, 0 */
  (B4 | B3 | B2),                           /* 73, 4, 3, 2, 0 */
  (B7 | B4 | B3),                           /* 74, 7, 4, 3, 0 */
  (B6 | B3 | B1),                           /* 75, 6, 3, 1, 0 */
  (B5 | B4 | B2),                           /* 76, 5, 4, 2, 0 */
  (B6 | B5 | B2),                           /* 77, 6, 5, 2, 0 */
  (B7 | B2 | B1),                           /* 78, 7, 2, 1, 0 */
  (B4 | B3 | B2),                           /* 79, 4, 3, 2, 0 */
  (B7 | B5 | B3 | B2 | B1),                 /* 80, 7, 5, 3, 2, 1, 0 */
  (B4),                                     /* 81, 4, 0 */
  (B8 | B7 | B6 | B4 | B1),                 /* 82, 8, 7, 6, 4, 1, 0 */
  (B7 | B4 | B2),                           /* 83, 7, 4, 2, 0 */
  (B8 | B7 | B5 | B3 | B1),                 /* 84, 8, 7, 5, 3, 1, 0 */
  (B8 | B2 | B1),                           /* 85, 8, 2, 1, 0 */
  (B6 | B5 | B2),                           /* 86, 6, 5, 2, 0 */
  (B7 | B5 | B1),                           /* 87, 7, 5, 1, 0 */
  (B8 | B5 | B4 | B3 | B1),                 /* 88, 8, 5, 4, 3, 1, 0 */
  (B6 | B5 | B3),                           /* 89, 6, 5, 3, 0 */
  (B5 | B3 | B2),                           /* 90, 5, 3, 2, 0 */
  (B7 | B6 | B5 | B3 | B2),                 /* 91, 7, 6, 5, 3, 2, 0 */
  (B6 | B5 | B2),                           /* 92, 6, 5, 2, 0 */
  (B2),                                     /* 93, 2, 0 */
  (B6 | B5 | B1),                           /* 94, 6, 5, 1, 0 */
  (B6 | B5 | B4 | B2 | B1),                 /* 95, 6, 5, 4, 2, 1, 0 */
  (B7 | B6 | B4 | B3 | B2),                 /* 95, 7, 6, 4, 3, 2, 0 */
  0
} ;

/* allocate cyc_random */
cyc_random_t *alloc_cyc_random( int width ) {
  cyc_random_t *rd ;
  
  rd = (void *)calloc(1,sizeof(cyc_random_t)) ;
  rd->seed = mt_urandom(0) ;

  rd->width = width ;
  rd->rnd = alloc_mt_random() ;

  init_genrand(rd->seed, rd->rnd) ;

  return(rd) ;
  
}

/* deallocate random_mt */
void free_cyc_random(cyc_random_t *rd) {
  if( rd ) {
    if( rd->rnd ) free( rd->rnd ) ;
    free(rd) ;
  }
}

#define BITMASK(x) ( (uint64_t)0xffffffffffffffff >> (64 - (x)))

static void gen_cyc_random( cyc_random_t *rd ) {
  uint64_t t ;
  
  if( rd->data & 1 ) {
    t = rd->data ^ rd->ppdata ;
    rd->data = t >> 1 ;
    if( t & 1 ) rd->data |= rd->udata ;
  }
  else {
    rd->data = rd->data>>1  ;
  }
    
  if( rd->data == rd->zdata ) 
  {
    rd->next_zero = 1 ;
  }
  
  rd->rnd_data = rd->data ;
  
  if( rd->swap_point ) {
    t = (rd->rnd_data >> rd->swap_point) & (rd->mdata>> rd->swap_point) ;
    //printf( "swap %d orig %lx low %lx upper %lx\n", rd->swap_point, rd->rnd_data, t, (rd->rnd_data & BITMASK(rd->swap_point-1)) << rd->swap_point) ;
    t |= (rd->rnd_data & BITMASK(rd->swap_point-1)) << rd->swap_point ;
    rd->rnd_data = t & rd->mdata ;
  }

}


static void set_mask( cyc_random_t *rd ) {
  rd->ppdata = primpoli[rd->act_width-1] ;
  rd->mdata = BITMASK(rd->act_width) ;
  rd->udata = 1 << (rd->act_width - 1) ;
}

uint64_t cyc_random( cyc_random_t *rd )
{
  uint64_t result ;
  if( rd->act_width == 0 ) {
    rd->act_width = rd->width ;
    set_mask(rd) ;
  }
  rd->pdata = rd->data ;
  if( !rd->next_zero && 
      rd->data == rd->sdata ) 
  {
    rd->data = genrand(rd->rnd) ;
    rd->data = (rd->data << 32) | genrand(rd->rnd) ;
    rd->data = (rd->data << 32) | genrand(rd->rnd) ;
    rd->data = (rd->data << 32) | genrand(rd->rnd) ;
    rd->data &= rd->mdata ;
    while( rd->data == 0 ) {
      rd->data = genrand(rd->rnd) ;
      rd->data = (rd->data << 32) | genrand(rd->rnd) ;
      rd->data = (rd->data << 32) | genrand(rd->rnd) ;
      rd->data = (rd->data << 32) | genrand(rd->rnd) ;
      rd->data &= rd->mdata ;
    }
    rd->sdata = rd->pdata = rd->data ; // start, previous
    // point to inject zero
    rd->zdata = genrand(rd->rnd) ;
    rd->zdata = (rd->zdata << 32) | genrand(rd->rnd) ;
    rd->zdata = (rd->zdata << 32) | genrand(rd->rnd) ;
    rd->zdata = (rd->zdata << 32) | genrand(rd->rnd) ;
    rd->zdata &= rd->mdata ;
    while( rd->zdata == 0 ) {
      rd->zdata = genrand(rd->rnd) ;
      rd->zdata = (rd->zdata << 32) | genrand(rd->rnd) ;
      rd->zdata = (rd->zdata << 32) | genrand(rd->rnd) ;
      rd->zdata = (rd->zdata << 32) | genrand(rd->rnd) ;
      rd->zdata &= rd->mdata ;
    }
    rd->swap_point = 0 ; //genrand(rd->rnd) % rd->act_width ;
    /* debug */
    /*
      printf( "swap_point %d data %x\n", rd->swap_point, rd->data[0] ) ;
    */
  }
  if( rd->next_zero ) {
    rd->next_zero = 0 ;
    rd->rnd_data = 0 ;
  }
  else {
    gen_cyc_random(rd) ;
  }
  
  result = ( rd->rnd_data & rd->mdata );
  return result ;

}

#ifdef CYC_RANDOM_TEST
#define WIDTH 16
#define ARRAY_SIZE(x) ((uint64_t)1 << x)
void dump_cyc_random( cyc_random_t *rd )
{
  printf( "cyc_random seed %d width %d act %d next_zero %d split %d rnd_data %lx zdata %lx mdata %lx ppdata %lx udata %lx \n",
    rd->seed, rd->width, rd->act_width, rd->next_zero, rd->swap_point, rd->rnd_data, rd->zdata, rd->mdata, rd->ppdata, rd->udata
  ) ;
}
int main(int argc, char **argv)
{
  int i, j, k, l ;
  uint64_t data[ARRAY_SIZE(WIDTH)] ;
  uint64_t loop = ARRAY_SIZE(WIDTH) ;
  cyc_random_t *rd = alloc_cyc_random( WIDTH ) ;
  dump_cyc_random(rd) ;
  printf( "loop = %lx\n", loop) ;
  for( i = 0 ; i < 10 ; i++ ) {
    for( j = 0 ; j < loop ; j++ ) {
      uint64_t val = cyc_random(rd) ;
      dump_cyc_random(rd) ;
      data[j] = val ;
      for(k = 0 ; k < j ; k++ ) {
        if( data[k] == val ) {
          printf( "Error: %d %d %d data[%d] %lx == [%d] %lx\n", i, j, k, k, data[k], j, val ) ;
          for( l = 0 ; l < j ; l++ ){
            printf( "  data[%d] = %lx \n", l, data[l] ) ;
          }
          exit(1) ;        
        }
      }
    }

  }

}

#endif


