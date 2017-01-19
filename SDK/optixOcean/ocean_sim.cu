/*
 * Copyright 1993-2009 NVIDIA Corporation.  All rights reserved.
 * * NVIDIA Corporation and its licensors retain all intellectual property and * proprietary rights in and to this software and related documentation. 
 * Any use, reproduction, disclosure, or distribution of this software 
 * and related documentation without an express license agreement from
 * NVIDIA Corporation is strictly prohibited.
 *
 * Please refer to the applicable NVIDIA end user license agreement (EULA) 
 * associated with this source code for terms and conditions that govern 
 * your use of this NVIDIA software.
 * 
 */

///////////////////////////////////////////////////////////////////////////////

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
rtBuffer<float,  2>                    data;
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
      slope.x = data[ make_uint2( x+1, y   ) ]- data[ make_uint2( x-1, y   ) ];
      slope.y = data[ make_uint2( x,   y+1 ) ]- data[ make_uint2( x,   y-1 ) ];
    } else {
      slope = make_float2(0.0f, 0.0f);
    }
    float3 normal = normalize( cross( make_float3( 0.0f,          slope.y*height_scale, 2.0f / width ),
                                      make_float3( 2.0f / height, slope.x*height_scale, 0.0f         ) ) );
    normals[launch_index] = make_float4( normal, 0.0f );
}

