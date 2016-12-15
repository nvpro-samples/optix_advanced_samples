
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
#include "random.h"

using namespace optix;

rtDeclareVariable(rtObject,     top_object, , );
rtDeclareVariable(float,        scene_epsilon, , );
rtDeclareVariable(int,          max_depth, , );
rtDeclareVariable(unsigned int, radiance_ray_type, , );
rtDeclareVariable(float3, shading_normal, attribute shading_normal, ); 
rtDeclareVariable(float3, front_hit_point, attribute front_hit_point, );
rtDeclareVariable(float3, back_hit_point, attribute back_hit_point, );

rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );

rtDeclareVariable(float3,       cutoff_color, , );
rtDeclareVariable(float,        fresnel_exponent, , );
rtDeclareVariable(float,        fresnel_minimum, , );
rtDeclareVariable(float,        fresnel_maximum, , );
rtDeclareVariable(float,        refraction_index, , );
rtDeclareVariable(float3,       refraction_color, , );
rtDeclareVariable(float3,       reflection_color, , );
rtDeclareVariable(float3,       extinction_constant, , );

struct PerRayData_radiance
{
  float3 result;
  int depth;
  unsigned int seed;
};

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

// -----------------------------------------------------------------------------

RT_PROGRAM void closest_hit_radiance()
{
  // intersection vectors
  const float3 n = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal)); // normal
  const float3 fhp = rtTransformPoint(RT_OBJECT_TO_WORLD, front_hit_point);
  const float3 bhp = rtTransformPoint(RT_OBJECT_TO_WORLD, back_hit_point);

  // Refract and check for total internal reflection
  float3 transmission_direction;
  const bool tir = !( refract(transmission_direction, ray.direction, n, refraction_index) );

  // check for external or internal reflection
  const float cos_theta_i = dot(ray.direction, n);
  float cos_theta = 0.0f;
  if ( !tir ) {
      if (cos_theta_i < 0.0f) {
          cos_theta = -cos_theta_i;
      } else {
          cos_theta = dot(transmission_direction, n);
      }
  }

  const float reflection_weight = tir ? 1.0f : fresnel_schlick(cos_theta, fresnel_exponent, fresnel_minimum, fresnel_maximum);
  const float P = /*reflection_weight*/ tir ? 1.0f : 0.5f;
  const bool do_reflection = ( rnd( prd_radiance.seed ) <  P );

  float3 result = make_float3(0.0f);
  float3 color = cutoff_color;

  if ( do_reflection )  {
      if (prd_radiance.depth < max_depth) {
          const float3 r = reflect(ray.direction, n);
          color = TraceRay( fhp, r, prd_radiance );
      }
      result += ( reflection_weight / P ) * reflection_color * color;
  }
  else {  // refraction
      if (prd_radiance.depth < max_depth) {
          color = TraceRay(bhp, transmission_direction, prd_radiance);
      }
      result += ( ( 1.0f - reflection_weight ) / ( 1 - P ) )* refraction_color * color;
  }

  if( cos_theta_i > 0 ) {
    // Beer's Law attenuation when exiting surface
    result *= exp(extinction_constant * t_hit);
  }

  prd_radiance.result = result;
}


