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
#include "material_parameter.h"
#include "light_definition.h"
#include "shader_common.h"

// Context global variables provided by the renderer system.
rtDeclareVariable(rtObject, sysTopObject, , );
rtDeclareVariable(float,    sysSceneEpsilon, , );

// Semantic variables.
rtDeclareVariable(optix::Ray, theRay,                  rtCurrentRay, );
rtDeclareVariable(float,      theIntersectionDistance, rtIntersectionDistance, );

rtDeclareVariable(PerRayData, thePrd,                  rtPayload, );

// Attributes.
rtDeclareVariable(optix::float3, varGeoNormal, attribute GEO_NORMAL, );
//rtDeclareVariable(optix::float3, varTangent,   attribute TANGENT, );
rtDeclareVariable(optix::float3, varNormal,    attribute NORMAL, ); 
//rtDeclareVariable(optix::float3, varTexCoord,  attribute TEXCOORD, ); 

// Material parameter definition.
rtBuffer<MaterialParameter> sysMaterialParameters; // Context global buffer with an array of structures of MaterialParameter.
rtDeclareVariable(int,      parMaterialIndex, , ); // Per Material index into the sysMaterialParameters array.

rtBuffer<LightDefinition> sysLightDefinitions;
rtDeclareVariable(int,    sysNumLights, , );     // PERF Used many times and faster to read than sysLightDefinitions.size().

rtBuffer< rtCallableProgramId<void(float3 const& point, const float2 sample, LightSample& lightSample)> > sysSampleLight;


// Helper functions for sampling a cosine weighted hemisphere distrobution as needed for the Lambert shading model.

RT_FUNCTION void alignVector(float3 const& axis, float3& w)
{
  // Align w with axis.
  const float s = copysign(1.0f, axis.z);
  w.z *= s;
  const float3 h = make_float3(axis.x, axis.y, axis.z + s);
  const float  k = optix::dot(w, h) / (1.0f + fabsf(axis.z));
  w = k * h - w;
}

RT_FUNCTION void unitSquareToCosineHemisphere(const float2 sample, float3 const& axis, float3& w, float& pdf)
{
  // Choose a point on the local hemisphere coordinates about +z.
  const float theta = 2.0f * M_PIf * sample.x;
  const float r = sqrtf(sample.y);
  w.x = r * cosf(theta);
  w.y = r * sinf(theta);
  w.z = 1.0f - w.x * w.x - w.y * w.y;
  w.z = (0.0f < w.z) ? sqrtf(w.z) : 0.0f;
 
  pdf = w.z * M_1_PIf;

  // Align with axis.
  alignVector(axis, w);
}

RT_PROGRAM void closesthit()
{
  float3 geoNormal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, varGeoNormal));
  float3 normal    = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, varNormal));

  thePrd.pos      = theRay.origin + theRay.direction * theIntersectionDistance; // Advance the path to the hit position in world coordinates.

  // Explicitly include edge-on cases as frontface condition! (Important for nested materials shown in a later example.)
  thePrd.flags |= (0.0f <= optix::dot(thePrd.wo, geoNormal)) ? FLAG_FRONTFACE : 0;

  if ((thePrd.flags & FLAG_FRONTFACE) == 0) // Looking at the backface?
  {
    // Means geometric normal and shading normal are always defined on the side currently looked at.
    // This gives the backfaces of opaque BSDFs a defined result.
    geoNormal = -geoNormal;
    normal    = -normal;
    // Do not recalculate the frontface condition!
  }

  // A material system with support for arbitrary mesh lights would evaluate its emission here.
  // But since only parallelogram area lights are supported, those get a dedicated closest hit program to simplify this demo.
  thePrd.radiance = make_float3(0.0f);

  // Start fresh with the next BSDF sample.  (Either of these values remaining zero is an end-of-path condition.)
  thePrd.f_over_pdf = make_float3(0.0f);
  thePrd.pdf        = 0.0f;

  // Lambert sampling:
  // Cosine weighted hemisphere sampling above the shading normal.
  // This calculates the ray.direction for the next path segment in wi and its probability density function value in pdf.
  unitSquareToCosineHemisphere(rng2(thePrd.seed), normal, thePrd.wi, thePrd.pdf);

  // Do not sample opaque surfaces below the geometry!
  // Mind that the geometry normal has been flipped to the side the ray points at.
  if (thePrd.pdf <= 0.0f || optix::dot(thePrd.wi, geoNormal) <= 0.0f)
  {
    thePrd.flags |= FLAG_TERMINATE;
    return;
  }
  
  MaterialParameter parameters = sysMaterialParameters[parMaterialIndex];

  thePrd.f_over_pdf = parameters.albedo * (M_1_PIf * optix::dot(thePrd.wi, normal) / thePrd.pdf); // PERF wi and normal are in the same hemisphere, no fabsf() needed on the cosTheta.
  thePrd.flags     |= FLAG_DIFFUSE;

#if USE_NEXT_EVENT_ESTIMATION
  // Direct lighting if the sampled BSDF was diffuse and any light is in the scene.
  if ( /* (thePrd.flags & FLAG_DIFFUSE) && */ 0 < sysNumLights) // No need to check FLAG_DIFFUSE. That has been set one line above. See in optixIntro_07 when this is needed.
  {
    const float2 sample = rng2(thePrd.seed); // Use higher dimension samples for the position. (Irrelevant for the LCG).

    LightSample lightSample; // Sample one of many lights. 
    
    // The caller picks the light to sample. Make sure the index stays in the bounds of the sysLightDefinitions array.
    lightSample.index = optix::clamp(static_cast<int>(floorf(rng(thePrd.seed) * sysNumLights)), 0, sysNumLights - 1); 

    const LightType lightType = sysLightDefinitions[lightSample.index].type;

    sysSampleLight[lightType](thePrd.pos, sample, lightSample); // lightSample direction and distance returned in world space!
  
    if (0.0f < lightSample.pdf) // Useful light sample?
    {
      // Lambert evaluation
      // Evaluate the Lambert BSDF in the light sample direction. Normally cheaper than shooting rays.
      const float3 f   = parameters.albedo * M_1_PIf;
      const float  pdf = fmaxf(0.0f, optix::dot(lightSample.direction, normal) * M_1_PIf);

      if (0.0f < pdf && isNotNull(f))
      {
        // Do the visibility check of the light sample.
        PerRayData_shadow prdShadow;

        prdShadow.visible = true; // Initialize for miss.

        // Note that the sysSceneEpsilon is applied on both sides of the shadow ray [t_min, t_max] interval 
        // to prevent self intersections with the actual light geometry in the scene!
        optix::Ray ray = optix::make_Ray(thePrd.pos, lightSample.direction, 1, sysSceneEpsilon, lightSample.distance - sysSceneEpsilon); // Shadow ray.
        rtTrace(sysTopObject, ray, prdShadow);

        if (prdShadow.visible)
        {
          const float misWeight = powerHeuristic(lightSample.pdf, pdf);
          
          thePrd.radiance += f * lightSample.emission * (misWeight * optix::dot(lightSample.direction, normal) / lightSample.pdf);
        }
      }
    }
  }
#endif // USE_NEXT_EVENT_ESTIMATION
}
