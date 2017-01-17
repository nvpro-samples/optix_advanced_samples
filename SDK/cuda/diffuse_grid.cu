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

rtDeclareVariable( float3, shading_normal, attribute shading_normal, ); 
rtDeclareVariable( float3, geometric_normal, attribute geometric_normal, );
rtDeclareVariable( float3, front_hit_point, attribute front_hit_point, );
rtDeclareVariable( float3, texcoord, attribute texcoord, );

rtDeclareVariable(optix::Ray, ray,   rtCurrentRay, );
rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );

rtTextureSampler<float4, 2> Kd_map;
rtDeclareVariable( float2, Kd_map_scale, , );

RT_PROGRAM void closest_hit_radiance()
{

    const float3 world_shading_normal   = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal ) );
    const float3 world_geometric_normal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );
    const float3 ffnormal = faceforward( world_shading_normal, -ray.direction, world_geometric_normal );

    const float z1 = rnd( prd_radiance.seed );
    const float z2 = rnd( prd_radiance.seed );
    
    float3 w_in;
    cosine_sample_hemisphere( z1, z2, w_in );
    const optix::Onb onb( ffnormal );
    onb.inverse_transform( w_in );
    const float3 fhp = rtTransformPoint( RT_OBJECT_TO_WORLD, front_hit_point );

    prd_radiance.origin = front_hit_point;
    prd_radiance.direction = w_in;
    
    const float3 Kd = make_float3( tex2D( Kd_map, texcoord.x / Kd_map_scale.x, texcoord.y / Kd_map_scale.y ) );
    prd_radiance.attenuation *= Kd;

}

