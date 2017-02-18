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
#include "ppm.h"
#include "random.h"

using namespace optix;

//
// Scene wide variables
//
rtDeclareVariable(float,         scene_epsilon, , );
rtDeclareVariable(rtObject,      top_object, , );


//
// Ray generation program
//
rtBuffer<PhotonRecord, 1>        ppass_output_buffer;
rtBuffer<uint2, 2>               photon_rnd_seeds;
rtDeclareVariable(uint,          max_depth, , );
rtDeclareVariable(uint,          max_photon_count, , );
rtDeclareVariable(PPMLight,      light , , );

rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );


static __device__ __inline__ float2 rnd_from_uint2( uint2& prev )
{
  return make_float2(rnd(prev.x), rnd(prev.y));
}

static __device__ __inline__ void mapToDisk( optix::float2& sample )
{
  float phi, r;
  float a = 2.0f * sample.x - 1.0f;      // (a,b) is now on [-1,1]^2 
  float b = 2.0f * sample.y - 1.0f;      // 
  if (a > -b) {                           // reg 1 or 2 
    if (a > b) {                          // reg 1, also |a| > |b| 
      r = a;
      phi = (M_PIf/4.0f) * (b/a);
    } else {                              // reg 2, also |b| > |a| 
      r = b;
      phi = (M_PIf/4.0f) * (2.0f - a/b);
    }
  } else {                                // reg 3 or 4 
    if (a < b) {                          // reg 3, also |a|>=|b| && a!=0 
      r = -a;
      phi = (M_PIf/4.0f) * (4.0f + b/a);
    } else {                              // region 4, |b| >= |a|,  but 
      // a==0 and  b==0 could occur. 
      r = -b;
      phi = b != 0.0f ? (M_PIf/4.0f) * (6.0f - a/b) :
        0.0f;
    }
  }
  float u = r * cosf( phi );
  float v = r * sinf( phi );
  sample.x = u;
  sample.y = v;
}

// sample hemisphere with cosine density
static __device__ __inline__ void sampleUnitHemisphere( const optix::float2& sample,
                                                 const optix::float3& U,
                                                 const optix::float3& V,
                                                 const optix::float3& W,
                                                 optix::float3& point )
{
    float phi = 2.0f * M_PIf*sample.x;
    float r = (float)sqrt( sample.y );
    float x = r * (float)cos(phi);
    float y = r * (float)sin(phi);
    float z = 1.0f - x*x -y*y;
    z = z > 0.0f ? (float)sqrt(z) : 0.0f;

    point = x*U + y*V + z*W;
}

static __device__ __inline__ void generateAreaLightPhoton( const PPMLight& light, const float2& d_sample, float3& o, float3& d)
{
  // Choose a random position on light
  o = light.anchor + 0.5f * ( light.v1 + light.v2);
  
  // Choose a random direction from light
  float3 U, V, W;
  create_onb( light.direction, U, V, W);
  sampleUnitHemisphere( d_sample, U, V, W, d );
}

static __device__ __inline__ void generateSpotLightPhoton( const PPMLight& light, const float2& d_sample, float3& o, float3& d)
{
  o = light.position;

  // Choose random dir by sampling disk of radius light.radius and projecting up to unit hemisphere
  float2 square_sample = d_sample; 
  mapToDisk( square_sample );
  square_sample = square_sample * atanf( light.radius );
  float x = square_sample.x;
  float y = square_sample.y;
  float z = sqrtf( fmaxf( 0.0f, 1.0f - x*x - y*y ) );

  // Now transform into light space
  float3 U, V, W;
  create_onb(light.direction, U, V, W);
  d =  x*U + y*V + z*W;
}


RT_PROGRAM void ppass_camera()
{
  size_t2 size     = photon_rnd_seeds.size();
  uint    pm_index = (launch_index.y * size.x + launch_index.x) * max_photon_count;
  uint2   seed     = photon_rnd_seeds[launch_index]; // No need to reset since we dont reuse this seed

  float2 direction_sample = make_float2(
      ( static_cast<float>( launch_index.x ) + rnd( seed.x ) ) / static_cast<float>( size.x ),
      ( static_cast<float>( launch_index.y ) + rnd( seed.y ) ) / static_cast<float>( size.y ) );
  float3 ray_origin, ray_direction;
  if( light.is_area_light ) {
    generateAreaLightPhoton( light, direction_sample, ray_origin, ray_direction );
  } else {
    generateSpotLightPhoton( light, direction_sample, ray_origin, ray_direction );
  }

  optix::Ray ray(ray_origin, ray_direction, ppass_and_gather_ray_type, scene_epsilon );

  // Initialize our photons
  for(unsigned int i = 0; i < max_photon_count; ++i) {
    ppass_output_buffer[i+pm_index].energy = make_float3(0.0f);
  }

  PhotonPRD prd;
  //  rec.ray_dir = ray_direction; // set in ppass_closest_hit
  prd.energy = light.power;
  prd.sample = seed;
  prd.pm_index = pm_index;
  prd.num_deposits = 0;
  prd.ray_depth = 0;
  rtTrace( top_object, ray, prd );
}

//
// Closest hit material
//
rtDeclareVariable(float3,  Ks, , );
rtDeclareVariable(float3,  Kd, , );
rtDeclareVariable(float3,  emitted, , );
rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, ); 
rtDeclareVariable(float3, shading_normal, attribute shading_normal, ); 
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );
rtDeclareVariable(PhotonPRD, hit_record, rtPayload, );

RT_PROGRAM void ppass_closest_hit()
{
  // Check if this is a light source
  float3 world_shading_normal   = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal ) );
  float3 world_geometric_normal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );
  float3 ffnormal     = faceforward( world_shading_normal, -ray.direction, world_geometric_normal );

  float3 hit_point = ray.origin + t_hit*ray.direction;
  float3 new_ray_dir;

  if( fmaxf( Kd ) > 0.0f ) {
    // We hit a diffuse surface; record hit if it has bounced at least once
    if( hit_record.ray_depth > 0 ) {
      PhotonRecord& rec = ppass_output_buffer[hit_record.pm_index + hit_record.num_deposits];
      rec.position = hit_point;
      rec.normal = ffnormal;
      rec.ray_dir = ray.direction;
      rec.energy = hit_record.energy;
      hit_record.num_deposits++;
    }

    hit_record.energy = Kd * hit_record.energy; 
    float3 U, V, W;
    create_onb(ffnormal, U, V, W);
    sampleUnitHemisphere(rnd_from_uint2(hit_record.sample), U, V, W, new_ray_dir);

  } else {
    hit_record.energy = Ks * hit_record.energy;
    // Make reflection ray
    new_ray_dir = reflect( ray.direction, ffnormal );
  }

  hit_record.ray_depth++;
  if ( hit_record.num_deposits >= max_photon_count || hit_record.ray_depth >= max_depth)
    return;

  optix::Ray new_ray( hit_point, new_ray_dir, ppass_and_gather_ray_type, scene_epsilon );
  rtTrace(top_object, new_ray, hit_record);
}

