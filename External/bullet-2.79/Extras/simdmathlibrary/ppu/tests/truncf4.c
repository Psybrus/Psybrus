/* Testcase for truncf4
   Copyright (C) 2006, 2007 Sony Computer Entertainment Inc.
   All rights reserved.

   Redistribution and use in source and binary forms,
   with or without modification, are permitted provided that the
   following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Sony Computer Entertainment Inc nor the names
      of its contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "common-test.h"
#include "testutils.h"
#include "simdmath.h"


int main()
{
   TEST_SET_START("20040916100012EJL","EJL", "truncf");
   
   unsigned int i3 = 0x4affffff;  // 2^23 - 0.5, largest truncatable value.
   unsigned int i3i = 0x4afffffe;  
   unsigned int i4 = 0x4b000000;  // 2^23, no fractional part.
   unsigned int i5 = 0xcf000001;  // -2^31, one more large, and negative, value.

   float x0 = hide_float(0.91825f);
   float x0i = hide_float(0.0f);
   float x1 = hide_float(-0.12958f);
   float x1i = hide_float(0.0f);
   float x2 = hide_float(-79615.1875f);
   float x2i = hide_float(-79615.0f);
   float x3 = hide_float(make_float(i3));
   float x3i = hide_float(make_float(i3i));
   float x4 = hide_float(make_float(i4));
   float x4i = hide_float(make_float(i4));
   float x5 = hide_float(make_float(i5));
   float x5i = hide_float(make_float(i5));

   vec_float4 x0_v = vec_splat_float(x0);
   vec_float4 x0i_v = vec_splat_float(x0i);
   vec_float4 x1_v = vec_splat_float(x1);
   vec_float4 x1i_v = vec_splat_float(x1i);
   vec_float4 x2_v = vec_splat_float(x2);
   vec_float4 x2i_v = vec_splat_float(x2i);
   vec_float4 x3_v = vec_splat_float(x3);
   vec_float4 x3i_v = vec_splat_float(x3i);
   vec_float4 x4_v = vec_splat_float(x4);
   vec_float4 x4i_v = vec_splat_float(x4i);
   vec_float4 x5_v = vec_splat_float(x5);
   vec_float4 x5i_v = vec_splat_float(x5i);
   
   vec_float4 res_v;

   TEST_START("truncf4");
   res_v = truncf4(x0_v);
   TEST_CHECK("20040916100023EJL", allequal_float4( res_v, x0i_v ), 0);
   res_v = truncf4(x1_v);
   TEST_CHECK("20040916100034EJL", allequal_float4( res_v, x1i_v ), 0);
   res_v = truncf4(x2_v);
   TEST_CHECK("20040916100043EJL", allequal_float4( res_v, x2i_v ), 0);
   res_v = truncf4(x3_v);
   TEST_CHECK("20040916100054EJL", allequal_float4( res_v, x3i_v ), 0);
   res_v = truncf4(x4_v);
   TEST_CHECK("20040916100103EJL", allequal_float4( res_v, x4i_v ), 0);
   res_v = truncf4(x5_v);
   TEST_CHECK("20040916100111EJL", allequal_float4( res_v, x5i_v ), 0);
   
   TEST_SET_DONE();
   
   TEST_EXIT();
}
