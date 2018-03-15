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
#include "material_parameter.h"
#include "per_ray_data.h"
#include "shader_common.h"

rtDeclareVariable(optix::Ray, theRay,                  rtCurrentRay, );
rtDeclareVariable(float,      theIntersectionDistance, rtIntersectionDistance, );

rtDeclareVariable(PerRayData_shadow, thePrdShadow, rtPayload, );

// Attributes.
//rtDeclareVariable(optix::float3, varGeoNormal, attribute GEO_NORMAL, );
//rtDeclareVariable(optix::float3, varTangent,   attribute TANGENT, );
//rtDeclareVariable(optix::float3, varNormal,    attribute NORMAL, ); 
rtDeclareVariable(optix::float3, varTexCoord,  attribute TEXCOORD, ); 

// Material parameter definition.
rtBuffer<MaterialParameter> sysMaterialParameters; // Context global buffer with an array of structures of MaterialParameter.
rtDeclareVariable(int,      parMaterialIndex, , ); // Per Material index into the above sysMaterialParameters array.


// One anyhit program for the radiance ray for all materials with cutout opacity!
RT_PROGRAM void anyhit_cutout() // For the radiance ray type.
{
  float opacity = 1.0f;
  const int id = sysMaterialParameters[parMaterialIndex].cutoutID; // Fetch the bindless texture ID for cutout opacity.
  if (id != RT_TEXTURE_ID_NULL)
  {
    opacity = intensity(make_float3(optix::rtTex2D<float4>(id, varTexCoord.x, varTexCoord.y))); // RGB intensity defines the opacity. White is opaque.
  }

  // Stochastic alpha test to get an alpha blend effect.
  if (opacity < 1.0f && opacity <= rng(thePrdShadow.seed)) // No need to calculate an expensive random number if the test is going to fail anyway.
  {
    rtIgnoreIntersection();
  }
}


// The shadow ray program for all materials with no cutout opacity.
RT_PROGRAM void anyhit_shadow()
{
  thePrdShadow.visible = false;
  rtTerminateRay();
}

RT_PROGRAM void anyhit_shadow_cutout() // For the shadow ray type.
{
  float opacity = 1.0f;
  const int id = sysMaterialParameters[parMaterialIndex].cutoutID; // Fetch the bindless texture ID for cutout opacity.
  if (id != RT_TEXTURE_ID_NULL)
  {
    opacity = intensity(make_float3(optix::rtTex2D<float4>(id, varTexCoord.x, varTexCoord.y))); // RGB intensity defines the opacity. White is opaque.
  }

  // Stochastic alpha test to get an alpha blend effect.
  if (opacity < 1.0f && opacity <= rng(thePrdShadow.seed)) // No need to calculate an expensive random number if the test is going to fail anyway.
  {
    rtIgnoreIntersection();
  }
  else
  {
    thePrdShadow.visible = false;
    rtTerminateRay();
  }
}

