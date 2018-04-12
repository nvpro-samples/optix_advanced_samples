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

RT_CALLABLE_PROGRAM void sample_bsdf_specular_reflection(MaterialParameter const& parameters, State const& state, PerRayData& prd)
{
  prd.wi = optix::reflect(-prd.wo, state.normal);

  if (optix::dot(prd.wi, state.geoNormal) <= 0.0f) // Do not sample opaque materials below the geometric surface.
  {
    prd.flags |= FLAG_TERMINATE;
    return;
  }

  prd.f_over_pdf = parameters.albedo;
  prd.pdf        = 1.0f; // Not 0.0f to make sure the path is not terminated. Otherwise unused for specular events.
}

// This is actually never reached, because the FLAG_DIFFUSE flag is not set when a specular BSDF is has been sampled.
RT_CALLABLE_PROGRAM float4 eval_bsdf_specular_reflection(MaterialParameter const& parameters, State const& state, PerRayData const& prd, float3 const& wiL)
{
  return make_float4(0.0f);
}

