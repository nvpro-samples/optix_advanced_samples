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

  thePrd.pos = theRay.origin + theRay.direction * theIntersectionDistance; // Advance the path to the hit position in world coordinates.

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
  thePrd.radiance = make_float3(0.0f);

  // Start fresh with the next BSDF sample.  (Either of these values remaining zero is an end-of-path condition.)
  thePrd.f_over_pdf = make_float3(0.0f);
  thePrd.pdf        = 0.0f;

  // Lambert sampling: Cosine weighted hemisphere sampling above the shading normal.
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

  // This would be the universal implementation for an arbitrary sampling of a diffuse surface.
  // thePrd.f_over_pdf = parameters.albedo * (M_1_PIf * fabsf(optix::dot(prd.wi, normal)) / prd.pdf); 
  
  // PERF Since the cosine-weighted hemisphere distribution is a perfect importance-sampling of the Lambert material,
  // the whole term ((M_1_PIf * fabsf(optix::dot(prd.wi, normal)) / prd.pdf) is always 1.0f here!
  thePrd.f_over_pdf = parameters.albedo;

  // This is a brute-force path tracer. There is no next event estimation (direct lighting) here.
  // Note that because of that, the albedo affects the path throughput only.
  // This material is not returning any radiance because it's not a light source.
}
