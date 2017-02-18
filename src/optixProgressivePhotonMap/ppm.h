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

#include <optixu/optixu_math_namespace.h>

#define  PPM_X         ( 1 << 0 )
#define  PPM_Y         ( 1 << 1 )
#define  PPM_Z         ( 1 << 2 )
#define  PPM_LEAF      ( 1 << 3 )
#define  PPM_NULL      ( 1 << 4 )

#define  PPM_IN_SHADOW ( 1 << 5 )
#define  PPM_OVERFLOW  ( 1 << 6 )
#define  PPM_HIT       ( 1 << 7 )

enum RayTypes
{
    rtpass_ray_type,
    ppass_and_gather_ray_type,
    shadow_ray_type
};

struct PPMLight
{
  optix::uint   is_area_light;
  optix::float3 power;

  // For spotlight
  optix::float3 position;
  optix::float3 direction;
  float         radius;

  // Parallelogram
  optix::float3 anchor;
  optix::float3 v1;
  optix::float3 v2;
};

struct HitRecord
{
 // float3 ray_dir;          // rgp

  optix::float3 position;         //
  optix::float3 normal;           // Material shader
  optix::float3 attenuated_Kd;
  optix::uint   flags;

  float         radius2;          //
  float         photon_count;     // Client TODO: should be moved clientside?
  optix::float3 flux;             //
  float         accum_atten;
};


struct PackedHitRecord
{
  optix::float4 a;   // position.x, position.y, position.z, normal.x
  optix::float4 b;   // normal.y,   normal.z,   atten_Kd.x, atten_Kd.y
  optix::float4 c;   // atten_Kd.z, flags,      radius2,    photon_count
  optix::float4 d;   // flux.x,     flux.y,     flux.z,     accum_atten 
};


struct HitPRD
{
  optix::float3 attenuation;
  optix::uint   ray_depth;
};


struct PhotonRecord
{
  optix::float3 position;
  optix::float3 normal;      // Pack this into 4 bytes
  optix::float3 ray_dir;
  optix::float3 energy;
  optix::uint   axis;
  optix::float3 pad;
};


struct PackedPhotonRecord
{
  optix::float4 a;   // position.x, position.y, position.z, normal.x
  optix::float4 b;   // normal.y,   normal.z,   ray_dir.x,  ray_dir.y
  optix::float4 c;   // ray_dir.z,  energy.x,   energy.y,   energy.z
  optix::float4 d;   // axis,       padding,    padding,    padding
};


struct PhotonPRD
{
  optix::float3 energy;
  optix::uint2  sample;
  optix::uint   pm_index;
  optix::uint   num_deposits;
  optix::uint   ray_depth;
};


struct ShadowPRD
{
  float attenuation;
};
