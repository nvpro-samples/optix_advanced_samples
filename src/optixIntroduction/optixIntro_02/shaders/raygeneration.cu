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

#include "rt_assert.h"

rtBuffer<float4,  2> sysOutputBuffer; // RGBA32F

rtDeclareVariable(rtObject, sysTopObject, , );

rtDeclareVariable(uint2, theLaunchDim,   rtLaunchDim, );
rtDeclareVariable(uint2, theLaunchIndex, rtLaunchIndex, );

rtDeclareVariable(float3, sysCameraPosition, , );
rtDeclareVariable(float3, sysCameraU, , );
rtDeclareVariable(float3, sysCameraV, , );
rtDeclareVariable(float3, sysCameraW, , );


// Entry point for a pinhole camera.
RT_PROGRAM void raygeneration()
{
  PerRayData prd;

  prd.radiance = make_float3(0.0f);
  
  // The launch index is the pixel coordinate.
  // Note that launchIndex = (0, 0) is the bottom left corner of the image,
  // which matches the origin in the OpenGL texture used to display the result.
  const float2 pixel = make_float2(theLaunchIndex);
  // Sample the ray in the center of the pixel.
  const float2 fragment = pixel + make_float2(0.5f);
  // The launch dimension (set with rtContextLaunch) is the full client window in this demo's setup.
  const float2 screen = make_float2(theLaunchDim);
  // Normalized device coordinates in range [-1, 1].
  const float2 ndc = (fragment / screen) * 2.0f - 1.0f;

  const float3 origin    = sysCameraPosition;
  const float3 direction = optix::normalize(ndc.x * sysCameraU + ndc.y * sysCameraV + sysCameraW);

  // Shoot a ray from origin into direction (must always be normalized!) for ray type 0 and test the interval between 0.0 and RT_DEFAULT_MAX for intersections.
  // There is no geometry in the scene, yet, so this will always invoke the miss program assigned to ray type 0, which is the radiance ray in this implementation.
  optix::Ray ray = optix::make_Ray(origin, direction, 0, 0.0f, RT_DEFAULT_MAX);

  // Start the ray traversal at the scene's root node, which in this case is an empty Group.
  // The ray becomes the variable with rtCurrentRay semantic in the other program domains.
  // The PerRayData becomes the variable with the semantic rtPayload in the other program domains,
  // which allows to exchange arbitrary data between the program domains.
  rtTrace(sysTopObject, ray, prd);

  sysOutputBuffer[theLaunchIndex] = make_float4(prd.radiance, 1.0f);
}
