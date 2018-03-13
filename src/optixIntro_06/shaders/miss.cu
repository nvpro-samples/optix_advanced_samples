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

rtDeclareVariable(optix::Ray, theRay, rtCurrentRay, );

rtDeclareVariable(PerRayData, thePrd, rtPayload, );

rtBuffer<LightDefinition> sysLightDefinitions;

rtDeclareVariable(float, sysEnvironmentRotation, , );


// Not actually a light. Never appears inside the sysLightDefinitions.
RT_PROGRAM void miss_environment_null()
{
  thePrd.radiance = make_float3(0.0f);

  thePrd.flags |= FLAG_TERMINATE;
}

RT_PROGRAM void miss_environment_constant()
{
#if USE_NEXT_EVENT_ESTIMATION
  // If the last surface intersection was a diffuse which was directly lit with multiple importance sampling,
  // then calculate light emission with multiple importance sampling as well.
  const float weightMIS = (thePrd.flags & FLAG_DIFFUSE) ? powerHeuristic(thePrd.pdf, 0.25f * M_1_PIf) : 1.0f;
  thePrd.radiance = make_float3(weightMIS); // Constant white emission multiplied by MIS weight.
#else
  thePrd.radiance = make_float3(1.0f); // Constant white emission.
#endif

  thePrd.flags |= FLAG_TERMINATE;
}

