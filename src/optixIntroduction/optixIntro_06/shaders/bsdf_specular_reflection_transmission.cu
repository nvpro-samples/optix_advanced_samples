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


// This function evaluates a Fresnel dielectric function when the transmitting cosine ("cost")
// is unknown and the incident index of refraction is assumed to be 1.0f.
// \param et     The transmitted index of refraction.
// \param costIn The cosine of the angle between the incident direction and normal direction.
RT_FUNCTION float evaluateFresnelDielectric(const float et, const float cosIn)
{
  const float cosi = fabsf(cosIn);

  float sint = 1.0f - cosi * cosi;
  sint = (0.0f < sint) ? sqrtf(sint) / et : 0.0f;

  // Handle total internal reflection.
  if (1.0f < sint)
  {
    return 1.0f;
  }

  float cost = 1.0f - sint * sint;
  cost = (0.0f < cost) ? sqrtf(cost) : 0.0f;

  const float et_cosi = et * cosi;
  const float et_cost = et * cost;

  const float rPerpendicular = (cosi - et_cost) / (cosi + et_cost);
  const float rParallel      = (et_cosi - cost) / (et_cosi + cost);

  const float result = (rParallel * rParallel + rPerpendicular * rPerpendicular) * 0.5f;

  return (result <= 1.0f) ? result : 1.0f;
}

RT_CALLABLE_PROGRAM void sample_bsdf_specular_reflection_transmission(MaterialParameter const& parameters, State const& state, PerRayData& prd)
{
  // Return the current material's absorption coefficient and ior to the integrator to be able to support nested materials.
  prd.absorption_ior = make_float4(parameters.absorption, parameters.ior);

  // Need to figure out here which index of refraction to use if the ray is already inside some refractive medium.
  // This needs to happen with the original FLAG_FRONTFACE condition to find out from which side of the geometry we're looking!
  // ior.xy are the current volume's IOR and the surrounding volume's IOR.
  // Thin-walled materials have no volume, always use the frontface eta for them!
  const float eta = (prd.flags & (FLAG_FRONTFACE | FLAG_THINWALLED))
                    ? prd.absorption_ior.w / prd.ior.x 
                    : prd.ior.y / prd.absorption_ior.w;

  const float3 R = optix::reflect(-prd.wo, state.normal);

  float reflective = 1.0f;

  if (optix::refract(prd.wi, -prd.wo, state.normal, eta))
  {
    if (prd.flags & FLAG_THINWALLED)
    {
      prd.wi = -prd.wo; // Straight through, no volume.
    }
    // Total internal reflection will leave this reflection probability at 1.0f.
    reflective = evaluateFresnelDielectric(eta, optix::dot(prd.wo, state.normal));
  }
  
  const float pseudo = rng(prd.seed);
  if (pseudo < reflective)
  {
    prd.wi = R; // Fresnel reflection or total internal reflection.
  }
  else if (!(prd.flags & FLAG_THINWALLED)) // Only non-thinwalled materials have a volume and transmission events.
  {
    prd.flags |= FLAG_TRANSMISSION;
  }

  // No Fresnel factor here. The probability to pick one or the other side took care of that.
  prd.f_over_pdf = parameters.albedo;
  prd.pdf        = 1.0f;
}

// DAR PERF Same as every specular material.
// Save program code and use the function from bsdf_specular_reflection instead.
// It will never be reached (could set it to RT_PROGRAM_ID_NULL), because the material system only calls sysEvalBSDF for diffuse materials.
//RT_CALLABLE_PROGRAM float4 eval_bsdf_specular_reflection_transmission(MaterialParameter const& parameters, State const& state, PerRayData const& prd, float3 const& wiL)
//{
//  return make_float4(0.0f);
//}

