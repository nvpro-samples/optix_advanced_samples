
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
rtDeclareVariable(float3, shading_normal, attribute shading_normal, ); 
rtDeclareVariable(float3, front_hit_point, attribute front_hit_point, );
rtDeclareVariable(float3, back_hit_point, attribute back_hit_point, );

rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );

rtDeclareVariable(float3,       cutoff_color, , );
rtDeclareVariable(float,        refraction_index, , );
rtDeclareVariable(float3,       refraction_color, , );
rtDeclareVariable(float3,       reflection_color, , );
rtDeclareVariable(float3,       extinction_constant, , );

rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );

// -----------------------------------------------------------------------------

static __device__ __inline__ float3 TraceRay(float3 origin, float3 direction, PerRayData_radiance prd_in )
{
  optix::Ray ray = optix::make_Ray( origin, direction, radiance_ray_type, 0.0f, RT_DEFAULT_MAX );
  PerRayData_radiance prd;
  prd.depth = prd_in.depth+1;
  prd.seed = prd_in.seed;

  rtTrace( top_object, ray, prd );
  return prd.result;
}

static __device__ __inline__ float3 exp( const float3& x )
{
  return make_float3(exp(x.x), exp(x.y), exp(x.z));
}

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
        attenuation = exp(extinction_constant * t_hit);
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

    float3 traced_color = cutoff_color;

    const float z = rnd( prd_radiance.seed );
    if( z <= R ) {
        // Reflect
        if (prd_radiance.depth < max_depth) {
            const float3 w_in = optix::reflect( -w_out, normal ); 
            const float3 fhp = rtTransformPoint(RT_OBJECT_TO_WORLD, front_hit_point);
            traced_color = TraceRay( fhp, w_in, prd_radiance );
        }
        prd_radiance.result = reflection_color*attenuation*traced_color;
    } else {
        // Refract
        if (prd_radiance.depth < max_depth) {
            const float3 w_in = w_t;
            const float3 bhp = rtTransformPoint(RT_OBJECT_TO_WORLD, back_hit_point);
            traced_color = TraceRay( bhp, w_in, prd_radiance );
        }
        prd_radiance.result = refraction_color*attenuation*traced_color;
    }

}


