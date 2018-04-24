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

// Context global variables provided by the renderer system.
rtDeclareVariable(rtObject, sysTopObject, , );
//rtDeclareVariable(float,    sysSceneEpsilon, , );

// Semantic variables.
rtDeclareVariable(optix::Ray, theRay,                  rtCurrentRay, );
rtDeclareVariable(float,      theIntersectionDistance, rtIntersectionDistance, );

rtDeclareVariable(PerRayData, thePrd,                  rtPayload, );

// Attributes.
rtDeclareVariable(optix::float3, varGeoNormal, attribute GEO_NORMAL, );
//rtDeclareVariable(optix::float3, varTangent,   attribute TANGENT, );
//rtDeclareVariable(optix::float3, varNormal,    attribute NORMAL, ); 
//rtDeclareVariable(optix::float3, varTexCoord,  attribute TEXCOORD, ); 

rtBuffer<LightDefinition> sysLightDefinitions;
rtDeclareVariable(int,    parLightIndex, , );  // Index into the sysLightDefinitions array.

// Very simple closest hit program just for rectangle area lights.
RT_PROGRAM void closesthit_light()
{
  thePrd.pos      = theRay.origin + theRay.direction * theIntersectionDistance; // Advance the path to the hit position in world coordinates.

  const float3 geoNormal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, varGeoNormal)); // PERF Not really needed when it's known that light geometry is not under Transforms.

  const float cosTheta = optix::dot(thePrd.wo, geoNormal);
  thePrd.flags |= (0.0f <= cosTheta) ? FLAG_FRONTFACE : 0;

  thePrd.radiance = make_float3(0.0f); // Backside is black.

  if (thePrd.flags & FLAG_FRONTFACE) // Looking at the front face?
  {
    const LightDefinition light = sysLightDefinitions[parLightIndex];
    
    thePrd.radiance = light.emission;

#if USE_NEXT_EVENT_ESTIMATION
    const float pdfLight = (theIntersectionDistance * theIntersectionDistance) / (light.area * cosTheta); // Solid angle pdf. Assumes light.area != 0.0f.
    // If it's an implicit light hit from a diffuse scattering event and the light emission was not returning a zero pdf.
    if ((thePrd.flags & FLAG_DIFFUSE) && DENOMINATOR_EPSILON < pdfLight)
    {
      // Scale the emission with the power heuristic between the previous BSDF sample pdf and this implicit light sample pdf.
      thePrd.radiance *= powerHeuristic(thePrd.pdf, pdfLight);
    }
#endif // USE_NEXT_EVENT_ESTIMATION
  }

  // Lights have no other material properties than emission in this demo. Terminate the path.
  thePrd.flags |= FLAG_TERMINATE;
}

