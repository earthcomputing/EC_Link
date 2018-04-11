/* Mersenne Twister Pseudo-Random Number Generator             *
 *               by  Makoto Matsumoto and Takuji Nishimura     */
 
/* Note for Jeda port: 
 * This file contains both unsigned integer version and Real 
 * version of MT19937 (1999/10/28 version with the improved 
 * initialization code). 
 *
 * All the functions are declared as static to avoid possible
 * name conflict with other part (as this is linked with Verilog
 * and other venders PLI codes).
 *
 * Real version's name is changed as dsgenrand and dgenrand 
 *
 * Also, the random generator runs on a pointer to the random 
 * generation tructure. 
 * See ../include/random.h for the definition of the structure.
 *
 * This file is included in runtime/random.c but separated to 
 * clarify the boundary of the original development and maintains
 * the portablity to other programs.
 *
 * I'm very happy that I found this superior random function.
 * Great thanks to Makoto Matsumoto and Takuji Nishimura!
 *
 */

/* Additional note: 3/21/2002 
 *   Initialization of mt is improved on mt19937ar.c released on 1/26/2002.
 *   So it noe use the new initialization only if seed is used upon the
 *   initialization. If seed is omitted, it will generate the same sequence
 *   as before with original initialization scheme.
 *  The following is the copyright info from mt19937ar.c file.
 */
 
/* 
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)  
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.                          

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote 
        products derived from this software without specific prior written 
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.keio.ac.jp/matumoto/emt.html
   email: matumoto@math.keio.ac.jp
*/

/*************************************************************/
/* integer version                                           */
/*************************************************************/

/* A C-program for MT19937: Integer version (1999/10/28)          */
/*  genrand() generates one pseudorandom unsigned integer (32bit) */
/* which is uniformly distributed among 0 to 2^32-1  for each     */
/* call. sgenrand(seed) sets initial values to the working area   */
/* of 624 words. Before genrand(), sgenrand(seed) must be         */
/* called once. (seed is any 32-bit integer.)                     */
/*   Coded by Takuji Nishimura, considering the suggestions by    */
/* Topher Cooper and Marc Rieffel in July-Aug. 1997.              */

/* This library is free software; you can redistribute it and/or   */
/* modify it under the terms of the GNU Library General Public     */
/* License as published by the Free Software Foundation; either    */
/* version 2 of the License, or (at your option) any later         */
/* version.                                                        */
/* This library is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.            */
/* See the GNU Library General Public License for more details.    */
/* You should have received a copy of the GNU Library General      */
/* Public License along with this library; if not, write to the    */
/* Free Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA   */ 
/* 02111-1307  USA                                                 */

/* Copyright (C) 1997, 1999 Makoto Matsumoto and Takuji Nishimura. */
/* Any feedback is very welcome. For any question, comments,       */
/* see http://www.math.keio.ac.jp/matumoto/emt.html or email       */
/* matumoto@math.keio.ac.jp                                        */

/* REFERENCE                                                       */
/* M. Matsumoto and T. Nishimura,                                  */
/* "Mersenne Twister: A 623-Dimensionally Equidistributed Uniform  */
/* Pseudo-Random Number Generator",                                */
/* ACM Transactions on Modeling and Computer Simulation,           */
/* Vol. 8, No. 1, January 1998, pp 3--30.                          */

/* #include<stdio.h>  */

/* Period parameters */  
#define N 624
#define M 397
#define MATRIX_A 0x9908b0df   /* constant vector a */
#define UPPER_MASK 0x80000000 /* most significant w-r bits */
#define LOWER_MASK 0x7fffffff /* least significant r bits */

/* Tempering parameters */   
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y)  (y >> 11)
#define TEMPERING_SHIFT_S(y)  (y << 7)
#define TEMPERING_SHIFT_T(y)  (y << 15)
#define TEMPERING_SHIFT_L(y)  (y >> 18)


/* allocate the random structure, added by Atsushi Kasuya for Jeda port */
static mt_random_t *alloc_mt_random() {
  mt_random_t *rd ;
  
  rd = (mt_random_t *)calloc(1,sizeof(mt_random_t)) ;
  rd->mti = N+1 ;
  return (rd) ;
  
}

/* Initializing the array with a seed */
static void
sgenrand(unsigned long seed, mt_random_t *rd)
{
    int i;

    for (i=0;i<N;i++) {
         rd->mt[i] = seed & 0xffff0000;
         seed = 69069 * seed + 1;
         rd->mt[i] |= (seed & 0xffff0000) >> 16;
         seed = 69069 * seed + 1;
    }
    rd->mti = N;
}

/* initializes mt[N] with a seed */
static void init_genrand(unsigned long s, mt_random_t *rd)
{
    rd->mt[0]= s & 0xffffffffUL;
    for (rd->mti=1; rd->mti<N; rd->mti++) {
        rd->mt[rd->mti] = 
	  (1812433253UL * (rd->mt[rd->mti-1] ^ (rd->mt[rd->mti-1] >> 30)) 
	    + rd->mti); 
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        rd->mt[rd->mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}

/* Initialization by "sgenrand()" is an example. Theoretically,      */
/* there are 2^19937-1 possible states as an intial state.           */
/* This function allows to choose any of 2^19937-1 ones.             */
/* Essential bits in "seed_array[]" is following 19937 bits:         */
/*  (seed_array[0]&UPPER_MASK), seed_array[1], ..., seed_array[N-1]. */
/* (seed_array[0]&LOWER_MASK) is discarded.                          */ 
/* Theoretically,                                                    */
/*  (seed_array[0]&UPPER_MASK), seed_array[1], ..., seed_array[N-1]  */
/* can take any values except all zeros.                             */
void
lsgenrand(unsigned long *seed_array, mt_random_t *rd)
    /* the length of seed_array[] must be at least N */
{
    int i;

    for (i=0;i<N;i++) 
      rd->mt[i] = seed_array[i];
    rd->mti=N;
}

static unsigned long 
genrand(mt_random_t *rd)
{
    unsigned long y;
    static unsigned long mag01[2]={0x0, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (rd->mti >= N) { /* generate N words at one time */
        int kk;

        if (rd->mti == N+1)   /* if sgenrand() has not been called, */
            sgenrand(4357, rd); /* a default initial seed is used   */

        for (kk=0;kk<N-M;kk++) {
            y = (rd->mt[kk]&UPPER_MASK)|(rd->mt[kk+1]&LOWER_MASK);
            rd->mt[kk] = rd->mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        for (;kk<N-1;kk++) {
            y = (rd->mt[kk]&UPPER_MASK)|(rd->mt[kk+1]&LOWER_MASK);
            rd->mt[kk] = rd->mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        y = (rd->mt[N-1]&UPPER_MASK)|(rd->mt[0]&LOWER_MASK);
        rd->mt[N-1] = rd->mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1];

        rd->mti = 0;
    }
  
    y = rd->mt[rd->mti++];
    y ^= TEMPERING_SHIFT_U(y);
    y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
    y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
    y ^= TEMPERING_SHIFT_L(y);

    return y; 
}
