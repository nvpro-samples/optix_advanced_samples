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
#include "light_definition.h"
#include "shader_common.h"

#include "rt_assert.h"

rtBuffer<LightDefinition> sysLightDefinitions;
rtDeclareVariable(int,    sysNumLights, , ); // PERF Used many times and faster to read than sysLightDefinitions.size().


RT_FUNCTION void unitSquareToSphere(const float u, const float v, float3& p, float& pdf)
{
  p.z = 1.0f - 2.0f * u;
  float r = 1.0f - p.z * p.z;
  r = (0.0f < r) ? sqrtf(r) : 0.0f;
  
  const float phi = v * 2.0f * M_PIf;
  p.x = r * cosf(phi);
  p.y = r * sinf(phi);

  pdf = 0.25f * M_1_PIf;  // == 1.0f / (4.0f * M_PIf)
}

// Note that all light sampling routines return lightSample.direction and lightSample.distance in world space!

RT_CALLABLE_PROGRAM void sample_light_constant(float3 const& point, const float2 sample, LightSample& lightSample)
{
  unitSquareToSphere(sample.x, sample.y, lightSample.direction, lightSample.pdf);

  // Environment lights do not set the light sample position!
  lightSample.distance = RT_DEFAULT_MAX; // Environment light.

  // Explicit light sample. White scaled by inverse probabilty to hit this light.
  lightSample.emission = make_float3(sysNumLights);
}


RT_CALLABLE_PROGRAM void sample_light_parallelogram(float3 const& point, const float2 sample, LightSample& lightSample)
{
  lightSample.pdf = 0.0f; // Default return, invalid light sample (backface, edge on, or too near to the surface)

  const LightDefinition light = sysLightDefinitions[lightSample.index]; // The light index is picked by the caller!

  lightSample.position  = light.position + light.vecU * sample.x + light.vecV * sample.y; // The light sample position in world coordinates.
  lightSample.direction = lightSample.position - point; // Sample direction from surface point to light sample position.
  lightSample.distance  = optix::length(lightSample.direction);
  if (DENOMINATOR_EPSILON < lightSample.distance)
  {
    lightSample.direction /= lightSample.distance; // Normalized direction to light.
 
    const float cosTheta = optix::dot(-lightSample.direction, light.normal);
    if (DENOMINATOR_EPSILON < cosTheta) // Only emit light on the front side.
    {
      // Explicit light sample, must scale the emission by inverse probabilty to hit this light.
      lightSample.emission = light.emission * float(sysNumLights); 
      lightSample.pdf      = (lightSample.distance * lightSample.distance) / (light.area * cosTheta); // Solid angle pdf. Assumes light.area != 0.0f.
    }
  }
}
