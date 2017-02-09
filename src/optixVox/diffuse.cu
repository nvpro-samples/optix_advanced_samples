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
#include <optixu/optixu_math_namespace.h>
#include "helpers.h"
#include "prd.h"
#include "random.h"
#include "commonStructs.h"

using namespace optix;

rtDeclareVariable( float3, shading_normal, attribute shading_normal, ); 
rtDeclareVariable( float3, geometric_normal, attribute geometric_normal, );
rtDeclareVariable( float3, front_hit_point, attribute front_hit_point, );
rtDeclareVariable( float4, geometry_color, attribute geometry_color, );

rtDeclareVariable(optix::Ray, ray,   rtCurrentRay, );
rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );
rtDeclareVariable(PerRayData_shadow,   prd_shadow, rtPayload, );

rtDeclareVariable( float3, Kd, , );
rtDeclareVariable(rtObject,      top_object, , );

rtBuffer<DirectionalLight> light_buffer;

RT_PROGRAM void any_hit_shadow()
{
    prd_shadow.attenuation = make_float3( 0.0f );
    rtTerminateRay();
}

// Note: both the hemisphere and direct light sampling below use pure random numbers to avoid any patent issues.
// Stratified sampling or QMC would improve convergence.  Please keep this in mind when judging noise levels.

RT_PROGRAM void closest_hit_radiance()
{

    const float3 world_shading_normal   = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal ) );
    const float3 world_geometric_normal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );
    const float3 ffnormal = faceforward( world_shading_normal, -ray.direction, world_geometric_normal );

    const float z1 = rnd( prd_radiance.seed );
    const float z2 = rnd( prd_radiance.seed );
    
    float3 w_in;
    optix::cosine_sample_hemisphere( z1, z2, w_in );
    const optix::Onb onb( ffnormal );
    onb.inverse_transform( w_in );
    const float3 fhp = rtTransformPoint( RT_OBJECT_TO_WORLD, front_hit_point );

    prd_radiance.origin = front_hit_point;
    prd_radiance.direction = w_in;
    
    prd_radiance.attenuation *= Kd * make_float3( geometry_color );

    // Add direct light radiance modulated by shadow ray

    const DirectionalLight& light = light_buffer[0];
    const float3 light_center = fhp + light.direction;
    const float r1 = rnd( prd_radiance.seed );
    const float r2 = rnd( prd_radiance.seed );
    const float3 jittered_pos = light_center + light.radius*(2.0f*r1 - 1.0f)*light.v0 + light.radius*(2.0f*r2 - 1.0f)*light.v1;
    const float3 L = normalize( jittered_pos - fhp );

    const float NdotL = dot( ffnormal, L);
    if(NdotL > 0.0f) {
        PerRayData_shadow shadow_prd;
        shadow_prd.attenuation = make_float3( 1.0f );
        optix::Ray shadow_ray ( fhp, L, /*shadow ray type*/ 1, 0.0f );
        rtTrace(top_object, shadow_ray, shadow_prd);
        prd_radiance.radiance += NdotL * light.color * shadow_prd.attenuation;
    }
    

}

