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

#include "app_config.h"

#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include "rt_function.h"
#include "per_ray_data.h"
#include "shader_common.h"

#include "rt_assert.h"

rtBuffer<float4,  2> sysOutputBuffer; // RGBA32F

rtDeclareVariable(rtObject, sysTopObject, , );
rtDeclareVariable(float,    sysSceneEpsilon, , );
rtDeclareVariable(int2,     sysPathLengths, , );
rtDeclareVariable(int,      sysIterationIndex, , );

rtDeclareVariable(float3, sysCameraPosition, , );
rtDeclareVariable(float3, sysCameraU, , );
rtDeclareVariable(float3, sysCameraV, , );
rtDeclareVariable(float3, sysCameraW, , );

rtDeclareVariable(uint2, theLaunchDim,   rtLaunchDim, );
rtDeclareVariable(uint2, theLaunchIndex, rtLaunchIndex, );

RT_FUNCTION void integrator(PerRayData& prd, float3& radiance)
{
  radiance = make_float3(0.0f); // Start with black.
  
  float3 throughput = make_float3(1.0f); // The throughput for the next radiance, starts with 1.0f.

  int depth = 0; // Path segment index. Primary ray is 0.

  while (depth < sysPathLengths.y)
  {
    prd.wo    = -prd.wi; // wi is the next path segment ray.direction. wo is the direction to the observer.
    prd.flags = 0;       // Clear all non-persistent flags. None in this version.

    // Note that the primary rays wouldn't need to offset the ray t_min by sysSceneEpsilon.
    optix::Ray ray = optix::make_Ray(prd.pos, prd.wi, 0, sysSceneEpsilon, RT_DEFAULT_MAX);
    rtTrace(sysTopObject, ray, prd);

    radiance += throughput * prd.radiance;

    // Path termination by miss shader or sample() routines.
    // If terminate is true, f_over_pdf and pdf might be undefined.
    if ((prd.flags & FLAG_TERMINATE) || prd.pdf <= 0.0f || isNull(prd.f_over_pdf))
    {
      break;
    }

    // PERF f_over_pdf already contains the proper throughput adjustment for diffuse materials: f * (fabsf(optix::dot(prd.wi, state.normal)) / prd.pdf);
    throughput *= prd.f_over_pdf;

    // Russian Roulette path termination after a specified number of bounces in sysPathLengths.x would go here. See next examples.

    ++depth; // Next path segment.
  }
}

// Entry point for pinhole camera with manual accumulation, non-VCA.
RT_PROGRAM void raygeneration()
{
  PerRayData prd;

  // Initialize the random number generator seed from the linear pixel index and the iteration index.
  prd.seed = tea<8>(theLaunchIndex.y * theLaunchDim.x + theLaunchIndex.x, sysIterationIndex);

  // Pinhole camera implementation:
  // The launch index is the pixel coordinate.
  // Note that launchIndex = (0, 0) is the bottom left corner of the image,
  // which matches the origin in the OpenGL texture used to display the result.
  const float2 pixel = make_float2(theLaunchIndex);
  // Sample the ray in the center of the pixel.
  const float2 fragment = pixel + rng2(prd.seed); // Random jitter of the fragment location in this pixel.
  // The launch dimension (set with rtContextLaunch) is the full client window in this demo's setup.
  const float2 screen = make_float2(theLaunchDim);
  // Normalized device coordinates in range [-1, 1].
  const float2 ndc = (fragment / screen) * 2.0f - 1.0f;

  // The integrator expects the next path segments ray.origin in prd.pos and the next ray.direction in prd.wi.
  prd.pos = sysCameraPosition;
  prd.wi  = optix::normalize(ndc.x * sysCameraU + ndc.y * sysCameraV + sysCameraW);

  float3 radiance;

  integrator(prd, radiance); // In this case a unidirectional path tracer.

#if USE_DEBUG_EXCEPTIONS
  // DAR DEBUG Highlight numerical errors.
  if (isnan(radiance.x) || isnan(radiance.y) || isnan(radiance.z))
  {
    radiance = make_float3(1000000.0f, 0.0f, 0.0f); // super red
  }
  else if (isinf(radiance.x) || isinf(radiance.y) || isinf(radiance.z))
  {
    radiance = make_float3(0.0f, 1000000.0f, 0.0f); // super green
  }
  else if (radiance.x < 0.0f || radiance.y < 0.0f || radiance.z < 0.0f)
  {
    radiance = make_float3(0.0f, 0.0f, 1000000.0f); // super blue
  }
#else
  // NaN values will never go away. Filter them out before they can arrive in the output buffer.
  // This only has an effect if the debug coloring above is off!
  if (!(isnan(radiance.x) || isnan(radiance.y) || isnan(radiance.z)))
#endif
  {
    if (0 < sysIterationIndex)
    {
      float4 dst = sysOutputBuffer[theLaunchIndex];  // RGBA32F
      sysOutputBuffer[theLaunchIndex] = optix::lerp(dst, make_float4(radiance, 1.0f), 1.0f / (float) (sysIterationIndex + 1));
    }
    else
    {
      // sysIterationIndex 0 will fill the buffer.
      // If this isn't done separately, the result of the lerp() above is undefined, e.g. dst could be NaN.
      sysOutputBuffer[theLaunchIndex] = make_float4(radiance, 1.0f);
    }
  }
}

