/*
  https://github.com/banksean wrapped Makoto Matsumoto and Takuji Nishimura's code in a namespace
  so it's better encapsulated. Now you can have multiple random number generators
  and they won't stomp all over eachother's state.

  If you want to use this as a substitute for Math.random(), use the random()
  method like so:

  var m = new MersenneTwister();
  var randomNumber = m.random();

  You can also call the other genrand_{foo}() methods on the instance.

  If you want to use a specific seed in order to get a repeatable random
  sequence, pass an integer into the constructor:

  var m = new MersenneTwister(123);

  and that will always produce the same random sequence.

  Sean McCullough (banksean@gmail.com)

  Adapted from Python to C (email@seanreid.ca)
*/

/*
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_seed(seed)
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
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
*/

#include <tinymt/tinymt.h>

#include <malloc.h>
#include <string.h>

#define TINYMT_N    624
#define TINYMT_M    397
#define TINYMT_F    1812433253
#define TINYMT_U    11
#define TINYMT_S    7
#define TINYMT_B    0x9D2C5680
#define TINYMT_T    15
#define TINYMT_C    0xEFC60000
#define TINYMT_L    18

static __inline uint32_t tinymt_ror(uint32_t v, uint32_t bits) {
  return (v >> bits) | (v << (32 - bits));
}

void tinymt_init(tinymt_t* tinymt) {
  tinymt->state = (uint32_t*) calloc(TINYMT_N, sizeof(uint32_t));
  tinymt->index = TINYMT_N;
}

void tinymt_copy(tinymt_t* tinymt, const tinymt_t* other) {
  memcpy(tinymt->state, other->state, TINYMT_N * sizeof(uint32_t));
  tinymt->index = other->index;
}

void tinymt_destroy(tinymt_t* tinymt) {
  if(tinymt->state) {
    free(tinymt->state);
    tinymt->state = NULL;
  }
}

void tinymt_seed(tinymt_t* tinymt, int32_t s) {
	tinymt->state[0] = s;
  for(uint32_t i = 1; i < TINYMT_N; i++) {
    tinymt->state[i] = TINYMT_F * (tinymt->state[i - 1] ^ (tinymt->state[i - 1] >> 30)) + i;
  }
  tinymt->index = TINYMT_N;
}

void tinymt_twist(tinymt_t* tinymt) {
  for(uint32_t i = 0; i < TINYMT_N; i++) {
    uint32_t temp = (tinymt->state[i] & 0x80000000) + ((tinymt->state[(i + 1) % TINYMT_N]) & 0x7FFFFFFF);
    uint32_t tempShift = temp >> 1;
    if((temp & 1) != 0)
      tempShift = tempShift ^ 0x9908B0DF;
    tinymt->state[i] = tinymt->state[(i + TINYMT_M) % TINYMT_N] ^ tempShift;
  }
  tinymt->index = 0;
}

uint32_t tinymt_rand(tinymt_t* tinymt) {
  if(tinymt->index >= TINYMT_N)
    tinymt_twist(tinymt);

  uint32_t y = tinymt->state[tinymt->index++];
	y ^= y >> TINYMT_U;
	y ^= (y << TINYMT_S) & TINYMT_B;
	y ^= (y << TINYMT_T) & TINYMT_C;
	y ^= y >> TINYMT_L;

	return y;
}

uint32_t tinymt_rand_max(const tinymt_t* tinymt) {
  return 0xFFFFFFFF;
}
