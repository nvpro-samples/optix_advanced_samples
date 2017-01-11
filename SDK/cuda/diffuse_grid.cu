/*
 * Copyright (c) 2008 - 2009 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and proprietary
 * rights in and to this software, related documentation and any modifications thereto.
 * Any use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation is strictly
 * prohibited.
 *
 * TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS*
 * AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY
 * SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
 * BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR
 * INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES
 */

#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include "helpers.h"
#include "prd.h"
#include "random.h"

using namespace optix;

rtDeclareVariable(rtObject,     top_object, , );
rtDeclareVariable(int,          max_depth, , );
rtDeclareVariable(unsigned int, radiance_ray_type, , );
rtDeclareVariable( float3,      cutoff_color, , );
rtDeclareVariable( float,       frequency, , );

rtDeclareVariable(float3,  shading_normal, attribute shading_normal, ); 
rtDeclareVariable( float3, geometric_normal, attribute geometric_normal, );
rtDeclareVariable(float3,  front_hit_point, attribute front_hit_point, );
rtDeclareVariable( float3, texcoord, attribute texcoord, );

rtDeclareVariable(optix::Ray, ray,   rtCurrentRay, );
rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );

static __device__ __inline__ float3 TraceRay(float3 origin, float3 direction, PerRayData_radiance prd_in )
{
    optix::Ray ray = optix::make_Ray( origin, direction, radiance_ray_type, 0.0f, RT_DEFAULT_MAX );
    PerRayData_radiance prd;
    prd.depth = prd_in.depth+1;
    prd.seed = prd_in.seed;

    rtTrace( top_object, ray, prd );
    return prd.result;
}

RT_PROGRAM void closest_hit_radiance()
{

    const float3 world_shading_normal   = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal ) );
    const float3 world_geometric_normal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );
    const float3 ffnormal = faceforward( world_shading_normal, -ray.direction, world_geometric_normal );

    const float z1 = rnd( prd_radiance.seed );
    const float z2 = rnd( prd_radiance.seed );
    
    float3 traced_color = cutoff_color;
    if (prd_radiance.depth < max_depth) {
        float3 w_in;
        cosine_sample_hemisphere( z1, z2, w_in );
        optix::Onb onb( ffnormal );
        onb.inverse_transform( w_in );
        const float3 fhp = rtTransformPoint( RT_OBJECT_TO_WORLD, front_hit_point );
        traced_color = TraceRay( fhp, w_in, prd_radiance );
    }
    
    const float u = frequency * texcoord.x;
    const float uu = u - floorf( u );
    const float v = frequency * texcoord.y;
    const float vv = v - floorf( v );
    const float linewidth = 0.04f;
    const float halflinewidth = 0.5f*linewidth;
    const float gridval = 1.0f - fmaxf( 
        smoothstep( 0.5f - linewidth, 0.5f - halflinewidth, uu ) - smoothstep( 0.5f + halflinewidth, 0.5f + linewidth, uu ),
        smoothstep( 0.5f - linewidth, 0.5f - halflinewidth, vv ) - smoothstep( 0.5f + halflinewidth, 0.5f + linewidth, vv )
        );
    const float3 Kd = make_float3( gridval );
    prd_radiance.result = Kd * traced_color;

}

