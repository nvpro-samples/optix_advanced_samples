/* 
 * Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <optix.h>
#include <optix_math.h>
#include <cufft.h>
#include <math_constants.h>


rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );
rtDeclareVariable(uint2, launch_dim,   rtLaunchDim, );
rtDeclareVariable(float, patch_size,, );
rtDeclareVariable(float, t,, );
rtBuffer<float2, 2>                    h0;
rtBuffer<float2, 2>                    ht;
rtBuffer<float2, 2>                    ik_ht;


/******************************************************************************\
 * 
 * Frequency space spectrum generation 
 * 
\******************************************************************************/
// complex math functions
__device__
float2 conjugate(float2 arg)
{ return make_float2(arg.x, -arg.y); }

__device__
float2 complex_exp(float arg)
{ return make_float2(cosf(arg), sinf(arg)); }

__device__
float2 complex_add(float2 a, float2 b)
{ return make_float2(a.x + b.x, a.y + b.y); }

__device__
float2 complex_mult(float2 ab, float2 cd)
{ return make_float2(ab.x * cd.x - ab.y * cd.y, ab.x * cd.y + ab.y * cd.x); }

RT_PROGRAM void generate_spectrum()
{
    unsigned int x = launch_index.x; 
    unsigned int y = launch_index.y;
    
    // calculate coordinates
    float2 k;
    k.x = CUDART_PI_F * x / patch_size;
    k.y = 2.0f * CUDART_PI_F * y / patch_size;

    // calculate dispersion w(k)
    float k_len = sqrtf( k.x*k.x + k.y*k.y );
    float w = sqrtf( 9.81f * k_len );

    float2 h0_k  = h0[ make_uint2( x, y ) ];
    float2 h0_mk = h0[ make_uint2( x, launch_dim.y-1-y ) ];

    float2 h_tilda = complex_add( complex_mult(h0_k, complex_exp(w * t)),
                                  complex_mult(conjugate(h0_mk), complex_exp(-w * t)) );
    float2 ik_h_tilda = k*h_tilda;

    ht[ launch_index ] = h_tilda;
    ik_ht[ launch_index ] = ik_h_tilda;
}


/******************************************************************************\
 * 
 * Normal calculation 
 * 
\******************************************************************************/
rtBuffer<float,  2>                    heights;
rtBuffer<float4, 2>                    normals;

rtDeclareVariable(float, height_scale, , );

RT_PROGRAM void calculate_normals()
{
    unsigned int x = launch_index.x; 
    unsigned int y = launch_index.y;
    unsigned int width  = launch_dim.x;
    unsigned int height = launch_dim.y;

    float2 slope;
    if ( (x > 0u) && ( y > 0u ) && ( x < width-1u ) && ( y < height-1u ) ) {
      slope.x = heights[ make_uint2( x+1, y   ) ]- heights[ make_uint2( x-1, y   ) ];
      slope.y = heights[ make_uint2( x,   y+1 ) ]- heights[ make_uint2( x,   y-1 ) ];
    } else {
      slope = make_float2(0.0f, 0.0f);
    }
    float3 normal = normalize( cross( make_float3( 0.0f,          slope.y*height_scale, 2.0f / width ),
                                      make_float3( 2.0f / height, slope.x*height_scale, 0.0f         ) ) );
    normals[launch_index] = make_float4( normal, 0.0f );
}

