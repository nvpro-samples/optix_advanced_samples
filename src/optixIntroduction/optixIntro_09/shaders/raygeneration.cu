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

rtBuffer<float4, 2> sysOutputBuffer; // RGBA32F

#if USE_DENOISER
rtBuffer<float4, 2> sysAlbedoBuffer; // RGBA32F
#endif

rtDeclareVariable(rtObject, sysTopObject, , );
rtDeclareVariable(float,    sysSceneEpsilon, , );
rtDeclareVariable(int2,     sysPathLengths, , );
rtDeclareVariable(int,      sysIterationIndex, , );
rtDeclareVariable(int,      sysCameraType, , );
rtDeclareVariable(int,      sysShutterType, , );

// Bindless callable programs implementing different lens shaders.
rtBuffer< rtCallableProgramId<void(const float2 pixel, const float2 screen, const float2 sample, float3& origin, float3& direction)> > sysLensShader;

rtDeclareVariable(uint2, theLaunchDim,   rtLaunchDim, );
rtDeclareVariable(uint2, theLaunchIndex, rtLaunchIndex, );

RT_FUNCTION void integrator(PerRayData& prd, float3& radiance, float3& albedo)
{
  // This renderer supports nested volumes. Four levels is plenty enough for most cases.
  // The absorption coefficient and IOR of the volume the ray is currently inside.
  float4 absorptionStack[MATERIAL_STACK_SIZE]; // .xyz == absorptionCoefficient (sigma_a), .w == index of refraction

  radiance = make_float3(0.0f); // Start with black.
  albedo   = make_float3(0.0f); // Start with black.
  
  // case 0: Standard stochastic motion blur.
  float time = rng(prd.seed); // Set the time of this path to a random value in the range [0, 1).
  
  switch (sysShutterType) // In case another camera shutter is active reuse that random value.
  {
    case 1: // Rolling shutter from top to bottom. 
      // Note that launchIndex (0, 0) is as the bottom left corner, which matches what OpenGL expects as texture orientation.
      // Each row gets a different time plus some stochastic antialiasing on that line.
      time = (float(theLaunchDim.y - 1 - theLaunchIndex.y) + time) / float(theLaunchDim.y);
      break;
    case 2: // Rolling shutter from bottom to top. 
      time = (float(theLaunchIndex.y) + time) / float(theLaunchDim.y);
      break;
    case 3: // Rolling shutter from left to right.
      time = (float(theLaunchIndex.x) + time) / float(theLaunchDim.x);
      break;
    case 4: // Rolling shutter from right to left.
      time = (float(theLaunchDim.x - 1 - theLaunchIndex.x) + time) / float(theLaunchDim.x);
      break;
  }
        
  float3 throughput = make_float3(1.0f); // The throughput for the next radiance, starts with 1.0f.

  int stackIdx = MATERIAL_STACK_EMPTY; // Start with empty nested materials stack.
  int depth = 0;                       // Path segment index. Primary ray is 0.

  prd.absorption_ior = make_float4(0.0f, 0.0f, 0.0f, 1.0f); // Assume primary ray starts in vacuum.

  prd.flags = 0;

  // Russian Roulette path termination after a specified number of bounces needs the current depth.
  while (depth < sysPathLengths.y)
  {
    prd.wo        = -prd.wi;           // Direction to observer.
    prd.ior       = make_float2(1.0f); // Reset the volume IORs.
    prd.distance  = RT_DEFAULT_MAX;    // Shoot the next ray with maximum length.
    prd.flags    &= FLAG_CLEAR_MASK;   // Clear all non-persistent flags. In this demo only the last diffuse surface interaction stays.

    // Handle volume absorption of nested materials.
    if (MATERIAL_STACK_FIRST <= stackIdx) // Inside a volume?
    {
      prd.flags     |= FLAG_VOLUME;                            // Indicate that we're inside a volume. => At least absorption calculation needs to happen.
      prd.extinction = make_float3(absorptionStack[stackIdx]); // There is only volume absorption in this demo, no volume scattering.
      prd.ior.x      = absorptionStack[stackIdx].w;            // The IOR of the volume we're inside. Needed for eta calculations in transparent materials.
      if (MATERIAL_STACK_FIRST <= stackIdx - 1)
      {
        prd.ior.y = absorptionStack[stackIdx - 1].w; // The IOR of the surrounding volume. Needed when potentially leaving a volume to calculate eta in transparent materials.
      }
    }

    // Note that the primary rays (or volume scattering miss cases) wouldn't normally offset the ray t_min by sysSceneEpsilon. Keep it simple here.
    optix::Ray ray = optix::make_Ray(prd.pos, prd.wi, 0, sysSceneEpsilon, prd.distance);
    // Note that this time defines the semantic variable rtCurrentTime in the other program domains.
    rtTrace(sysTopObject, ray, time, prd); 

    // This renderer supports nested volumes.
    if (prd.flags & FLAG_VOLUME)
    {
      // We're inside a volume. Calculate the extinction along the current path segment in any case.
      // The transmittance along the current path segment inside a volume needs to attenuate the ray throughput with the extinction
      // before it modulates the radiance of the hitpoint.
      throughput *= expf(-prd.distance * prd.extinction);
    }

    radiance += throughput * prd.radiance;

#if USE_DENOISER
    // In physical terms, the albedo is a single color value approximating the ratio of radiant exitance to the irradiance under uniform lighting.
    // The albedo value can be approximated for simple materials by using the diffuse color of the first hit,
    // or for layered materials by using a weighted sum of the individual BRDFs’ albedo values.
    // For some objects such as perfect mirrors, the quality of the result might be improved by using the albedo value of a subsequent hit instead.
#if 1
    // When no albedo has been written before and the hit was diffuse or a light, write the albedo.
    // DAR This makes glass materials and motion blur on specular surfaces in the demo a little noisier,
    // but should definitely be used with high frequency textures behind transparent or around reflective materials.
    if (!(prd.flags & FLAG_ALBEDO) && (prd.flags & (FLAG_DIFFUSE | FLAG_LIGHT)))
    {
      // The albedo buffer should contain the surface appearance under uniform lighting in linear color space in the range [0.0f, 1.0f].
      // Clamp the final albedo result to that range here, because it captured the radiance when hitting lights either directly or via specular events.
      albedo = optix::clamp(throughput * prd.albedo, 0.0f, 1.0f);
      prd.flags |= FLAG_ALBEDO; // This flag is persistent along the path and prevents that the albedo is written more than once.
    }
#else    
    if (depth == 0) // Just write the albedo of the primary ray.
    {
      albedo = optix::clamp(throughput * prd.albedo, 0.0f, 1.0f); // See comment above. Expects linear colors in the range [0.0f, 1.0f]
    }
#endif

#endif // USE_DENOISER

    // Path termination by miss shader or sample() routines.
    // If terminate is true, f_over_pdf and pdf might be undefined.
    if ((prd.flags & FLAG_TERMINATE) || prd.pdf <= 0.0f || isNull(prd.f_over_pdf))
    {
      break;
    }

    // PERF f_over_pdf already contains the proper throughput adjustment for diffuse materials: f * (fabsf(optix::dot(prd.wi, state.normal)) / prd.pdf);
    throughput *= prd.f_over_pdf;

    // Unbiased Russian Roulette path termination.
    if (sysPathLengths.x <= depth) // Start termination after a minimum number of bounces.
    {
      const float probability = fmaxf(throughput); // DAR Other options: // intensity(throughput); // fminf(0.5f, intensity(throughput));
      if (probability < rng(prd.seed)) // Paths with lower probability to continue are terminated earlier.
      {
        break;
      }
      throughput /= probability; // Path isn't terminated. Adjust the throughput so that the average is right again.
    }

    // Adjust the material volume stack if the geometry is not thin-walled but a border between two volumes 
    // and the outgoing ray direction was a transmission.
    if ((prd.flags & (FLAG_THINWALLED | FLAG_TRANSMISSION)) == FLAG_TRANSMISSION) 
    {
      // Transmission.
      if (prd.flags & FLAG_FRONTFACE) // Entered a new volume?
      {
        // Push the entered material's volume properties onto the volume stack.
        //rtAssert((stackIdx < MATERIAL_STACK_LAST), 1); // Overflow?
        stackIdx = min(stackIdx + 1, MATERIAL_STACK_LAST);
        absorptionStack[stackIdx] = prd.absorption_ior;
      }
      else // Exited the current volume?
      {
        // Pop the top of stack material volume.
        // This assert fires and is intended because I tuned the frontface checks so that there are more exits than enters at silhouettes.
        //rtAssert((MATERIAL_STACK_EMPTY < stackIdx), 0); // Underflow?
        stackIdx = max(stackIdx - 1, MATERIAL_STACK_EMPTY);
      }
    }

    ++depth; // Next path segment.
  }
}

RT_PROGRAM void raygeneration()
{
  PerRayData prd;

  // Initialize the random number generator seed from the linear pixel index and the iteration index.
  prd.seed = tea<8>(theLaunchIndex.y * theLaunchDim.x + theLaunchIndex.x, sysIterationIndex);

  // DAR Decoupling the pixel coordinates from the screen size will allow for partial rendering algorithms.
  // In this case theLaunchIndex is the pixel coordinate and theLaunchDim is sysOutputBuffer.size().
  sysLensShader[sysCameraType](make_float2(theLaunchIndex), make_float2(theLaunchDim), rng2(prd.seed), prd.pos, prd.wi); // Calculate the primary ray with a lens shader program.

  float3 radiance;
  float3 albedo;

  integrator(prd, radiance, albedo); // In this case a unidirectional path tracer.

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

#if USE_DENOISER
      dst = sysAlbedoBuffer[theLaunchIndex];  // RGBA32F
      sysAlbedoBuffer[theLaunchIndex] = optix::lerp(dst, make_float4(albedo, 1.0f), 1.0f / (float) (sysIterationIndex + 1));
#endif
    }
    else
    {
      // sysIterationIndex 0 will fill the buffer.
      // If this isn't done separately, the result of the lerp() above is undefined, e.g. dst could be NaN.
      sysOutputBuffer[theLaunchIndex] = make_float4(radiance, 1.0f);
#if USE_DENOISER
      sysAlbedoBuffer[theLaunchIndex] = make_float4(albedo, 1.0f);
#endif
    }
  }
}

#if USE_DENOISER
// OptiX 5.0.x needs at least one appendLaunch() in the post-processing CommandList or the denoiser will not trigger its memory allocations.
// This is fixed in OptiX 5.1.0 which also supports HDR denoising directly, so that the tonemapper can be placed last again.
// Put my own tonemapper from GLSL here to get the proper gamma corrected input into the denoiser.

rtBuffer<float4, 2> sysTonemappedBuffer;

rtDeclareVariable(float3, sysColorBalance, , );
rtDeclareVariable(float,  sysInvGamma, , );
rtDeclareVariable(float,  sysInvWhitePoint, , );
rtDeclareVariable(float,  sysBurnHighlights, , );
rtDeclareVariable(float,  sysCrushBlacks, , );
rtDeclareVariable(float,  sysSaturation, , );

RT_PROGRAM void raygeneration_tonemapper()
{
  const float3 hdrColor = make_float3(sysOutputBuffer[theLaunchIndex]);

  float3 ldrColor = sysInvWhitePoint * sysColorBalance * hdrColor;
  ldrColor *= (ldrColor * make_float3(sysBurnHighlights) + make_float3(1.0f)) / (ldrColor + make_float3(1.0f));
  
  float luminance = optix::dot(ldrColor, make_float3(0.3f, 0.59f, 0.11f));
  ldrColor = optix::lerp(make_float3(luminance), ldrColor, sysSaturation); // This can generate negative values for sysSaturation > 1.0f!
  ldrColor = fmaxf(0.0f, ldrColor); // Prevent negative values.
  
  luminance = optix::dot(ldrColor, make_float3(0.3f, 0.59f, 0.11f));
  if (luminance < 1.0f)
  {
    const float3 crushed = powf(ldrColor, sysCrushBlacks);
    ldrColor = optix::lerp(crushed, ldrColor, sqrtf(luminance));
    ldrColor = fmaxf(0.0f, ldrColor); // Prevent negative values.
  }
  ldrColor = powf(ldrColor, sysInvGamma);

#if USE_DEBUG_EXCEPTIONS
  // DAR DEBUG Highlight numerical errors.
  if (isnan(ldrColor.x) || isnan(ldrColor.y) || isnan(ldrColor.z))
  {
    ldrColor = make_float3(1000000.0f, 0.0f, 0.0f); // super red
  }
  else if (isinf(ldrColor.x) || isinf(ldrColor.y) || isinf(ldrColor.z))
  {
    ldrColor = make_float3(0.0f, 1000000.0f, 0.0f); // super green
  }
  else if (ldrColor.x < 0.0f || ldrColor.y < 0.0f || ldrColor.z < 0.0f)
  {
    ldrColor = make_float3(0.0f, 0.0f, 1000000.0f); // super blue
  }
#endif

  sysTonemappedBuffer[theLaunchIndex] = make_float4(ldrColor, 1.0f);
}
#endif // USE_DENOISER

