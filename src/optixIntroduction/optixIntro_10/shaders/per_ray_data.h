/* 
 * Copyright (c) 2013-2018, NVIDIA CORPORATION. All rights reserved.
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

#pragma once

#ifndef PER_RAY_DATA_H
#define PER_RAY_DATA_H

#include "app_config.h"

#include "random_number_generators.h"

#define MATERIAL_STACK_EMPTY -1
#define MATERIAL_STACK_FIRST  0
#define MATERIAL_STACK_LAST   3
#define MATERIAL_STACK_SIZE   4

// Set when reaching a closesthit program. Unused in this demo
#define FLAG_HIT            0x00000001
// Set by BSDFs which support direct lighting. Not set means specular interaction. Cleared in the closesthit program.
// Used to decide when to do direct lighting and multuiple importance sampling on implicit light hits.
#define FLAG_DIFFUSE        0x00000002
// Set when a light was hit.
#define FLAG_LIGHT          0x00000004

// Set if (0.0f <= wo_dot_ng), means looking onto the front face. (Edge-on is explicitly handled as frontface for the material stack.)
#define FLAG_FRONTFACE      0x00000010
// Pass down material.flags through to the BSDFs.
#define FLAG_THINWALLED     0x00000020

// FLAG_TRANSMISSION is set if there is a transmission. (Can't happen when FLAG_THINWALLED is set.)
#define FLAG_TRANSMISSION   0x00000100
// Set if the material stack is not empty.
#define FLAG_VOLUME         0x00001000

// When using the AI denoiser the renderer builds an albedo buffer to improve the denoised result.
// This should only be written once and this flag can track if that happened. This flag is persistent along the path.
#define FLAG_ALBEDO         0x00010000

// Highest bit set means terminate path.
#define FLAG_TERMINATE      0x80000000

// Keep flags active in a path segment which need to be tracked along the path.
// In this case only the last surface interaction is kept.
// It's needed to track the last bounce's diffuse state in case a ray hits a light implicitly for multiple importance sampling.
// FLAG_DIFFUSE is reset in the closesthit program. 
#define FLAG_CLEAR_MASK     (FLAG_DIFFUSE | FLAG_ALBEDO)

// Currently only containing some vertex attributes in world coordinates.
struct State
{
  optix::float3 geoNormal;
  optix::float3 normal;
  optix::float3 texcoord;
};

// Note that the fields are ordered by CUDA alignment restrictions.
struct PerRayData
{
  optix::float4 absorption_ior; // The absorption coefficient and IOR of the currently hit material.
  optix::float2 ior;            // .x = IOR the ray currently is inside, .y = the IOR of the surrounding volume. The IOR of the current material is in absorption_ior.w!
  
  optix::float3 pos;            // Current surface hit point or volume sample point, in world space
  float         distance;       // Distance from the ray origin to the current position, in world space. Needed for absorption of nested materials.
  
  optix::float3 wo;             // Outgoing direction, to observer, in world space.
  optix::float3 wi;             // Incoming direction, to light, in world space.

  optix::float3 radiance;       // Radiance along the current path segment.
  int           flags;          // Bitfield with flags. See FLAG_* defines for its contents.

  optix::float3 f_over_pdf;     // BSDF sample throughput, pre-multiplied f_over_pdf = bsdf.f * fabsf(dot(wi, ns) / bsdf.pdf; 
  float         pdf;            // The last BSDF sample's pdf, tracked for multiple importance sampling.

  optix::float3 extinction;     // The current volume's extinction coefficient. (Only absorption in this implementation.)
  float         opacity;        // Cutout opacity result.

#if USE_DENOISER
#if USE_DENOISER_ALBEDO
  optix::float3 albedo;         // Albedo value to help the denoiser finding the correct result better.
#if USE_DENOISER_NORMAL
  optix::float3 normal;         // Shading normal for the denoiser's normal buffer. Only possible to use when albedo is used as well.
#endif
#endif
#endif

  unsigned int  seed;           // Random number generator input.
};

struct PerRayData_shadow
{
  unsigned int seed;
  bool         visible;
};

#endif // PER_RAY_DATA_H
