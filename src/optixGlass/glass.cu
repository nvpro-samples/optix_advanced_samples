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
#include "prd.h"
#include "random.h"

using namespace optix;

rtDeclareVariable(float3, shading_normal, attribute shading_normal, ); 
rtDeclareVariable(float3, front_hit_point, attribute front_hit_point, );
rtDeclareVariable(float3, back_hit_point, attribute back_hit_point, );

rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );

rtDeclareVariable(float,        refraction_index, , );
rtDeclareVariable(float3,       refraction_color, , );
rtDeclareVariable(float3,       reflection_color, , );
rtDeclareVariable(float3,       extinction_constant, , );

rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );

// -----------------------------------------------------------------------------

static __device__ __inline__ float fresnel( float cos_theta_i, float cos_theta_t, float eta )
{
    const float rs = ( cos_theta_i - cos_theta_t*eta ) / 
                     ( cos_theta_i + eta*cos_theta_t );
    const float rp = ( cos_theta_i*eta - cos_theta_t ) /
                     ( cos_theta_i*eta + cos_theta_t );

    return 0.5f * ( rs*rs + rp*rp );
}

// -----------------------------------------------------------------------------

RT_PROGRAM void closest_hit_radiance()
{
    const float3 w_out = -ray.direction;
    float3 normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal));
    float cos_theta_i = optix::dot( w_out, normal );

    float eta;
    float3 attenuation = make_float3( 1.0f );
    if( cos_theta_i > 0.0f ) {
        // Ray is entering 
        eta = refraction_index;  // Note: does not handle nested dielectrics
    } else {
        // Ray is exiting.
        attenuation = optix::expf(extinction_constant * t_hit);
        eta         = 1.0f / refraction_index;
        cos_theta_i = -cos_theta_i;
        normal      = -normal;
    }

    float3 w_t;
    const bool tir           = !optix::refract( w_t, -w_out, normal, eta );

    const float cos_theta_t  = -optix::dot( normal, w_t );
    const float R            = tir  ?
                               1.0f :
                               fresnel( cos_theta_i, cos_theta_t, eta );

    // Importance sample the Fresnel term
    const float z = rnd( prd_radiance.seed );
    if( z <= R ) {
        // Reflect
        const float3 w_in = optix::reflect( -w_out, normal ); 
        const float3 fhp = rtTransformPoint(RT_OBJECT_TO_WORLD, front_hit_point);
        prd_radiance.origin = fhp;
        prd_radiance.direction = w_in; 
        prd_radiance.attenuation *= reflection_color*attenuation;
    } else {
        // Refract
        const float3 w_in = w_t;
        const float3 bhp = rtTransformPoint(RT_OBJECT_TO_WORLD, back_hit_point);
        prd_radiance.origin = bhp;
        prd_radiance.direction = w_in; 
        prd_radiance.attenuation *= refraction_color*attenuation;
    }

    // Note: we do not trace the ray for the next bounce here, we just set it up for
    // the ray-gen program using per-ray data. 

}


