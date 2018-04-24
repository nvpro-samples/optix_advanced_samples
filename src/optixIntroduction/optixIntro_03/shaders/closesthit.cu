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

// Context global variables provided by the renderer system.
rtDeclareVariable(rtObject, sysTopObject, , );

// Semantic variables.
rtDeclareVariable(optix::Ray, theRay, rtCurrentRay, );

rtDeclareVariable(PerRayData, thePrd, rtPayload, );

// Attributes.
rtDeclareVariable(optix::float3, varGeoNormal, attribute GEO_NORMAL, );
//rtDeclareVariable(optix::float3, varTangent,   attribute TANGENT, );
rtDeclareVariable(optix::float3, varNormal,    attribute NORMAL, );
//rtDeclareVariable(optix::float3, varTexCoord,  attribute TEXCOORD, ); 

// This closest hit program only uses the geometric normal and the shading normal attributes.
// OptiX will remove all code from the intersection programs for unused attributes automatically.

// Note that the matching between attribute outputs from the intersection program and 
// the inputs in the closesthit and anyhit programs is done with the type (here float3) and
// the user defined attribute semantic (e.g. here NORMAL). 
// The actual variable name doesn't need to match but it's recommended for clarity.

RT_PROGRAM void closesthit()
{
  // Transform the (unnormalized) object space normals into world space.
  float3 geoNormal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, varGeoNormal));
  float3 normal    = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, varNormal));

  // Check if the ray hit the geometry on the frontface or the backface.
  // The geometric normal is always defined on the front face of the geometry.
  // In this implementation the coordinate systems are right-handed and the frontface triangle winding is counter-clockwise (matching OpenGL).

  // If theRay.direction and geometric normal are in the same hemisphere we're looking at a backface.
  if (0.0f < optix::dot(theRay.direction, geoNormal))
  {
    // Flip the shading normal to the backface, because only that is used below.
    // (See later examples for more intricate handling of the frontface condition.)
    normal = -normal;
  }

  // Visualize the resulting world space normal on the surface we're looking on.
  // Transform the normal components from [-1.0f, 1.0f] to the range [0.0f, 1.0f] to get colors for negative values.
  thePrd.radiance = normal * 0.5f + 0.5f;
}
