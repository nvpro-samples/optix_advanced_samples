/* 
 * Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.
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

#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include "helpers.h"
#include "prd.h"
#include "random.h"

using namespace optix;


rtDeclareVariable(float3,        eye, , );
rtDeclareVariable(float3,        U, , );
rtDeclareVariable(float3,        V, , );
rtDeclareVariable(float3,        W, , );
rtDeclareVariable(float3,        bad_color, , );
rtDeclareVariable(float,         scene_epsilon, , );
rtDeclareVariable(float3,        cutoff_color, , );
rtDeclareVariable(int,           max_depth, , );
rtBuffer<uchar4, 2>              output_buffer;
rtBuffer<float4, 2>              accum_buffer;
rtDeclareVariable(rtObject,      top_object, , );
rtDeclareVariable(unsigned int,  radiance_ray_type, , );
rtDeclareVariable(unsigned int,  frame, , );
rtDeclareVariable(uint2,         launch_index, rtLaunchIndex, );


RT_PROGRAM void pinhole_camera()
{

  size_t2 screen = output_buffer.size();
  unsigned int seed = tea<16>(screen.x*launch_index.y+launch_index.x, frame);

  // Subpixel jitter: send the ray through a different position inside the pixel each time,
  // to provide antialiasing.
  float2 subpixel_jitter = frame == 0 ? make_float2(0.0f, 0.0f) : make_float2(rnd( seed ) - 0.5f, rnd( seed ) - 0.5f);

  float2 d = (make_float2(launch_index) + subpixel_jitter) / make_float2(screen) * 2.f - 1.f;
  float3 ray_origin = eye;
  float3 ray_direction = normalize(d.x*U + d.y*V + W);

  PerRayData_radiance prd;
  prd.depth = 0;
  prd.seed = seed;
  prd.done = false;

  // These represent the current shading state and will be set by the closest-hit or miss program

  // brdf attenuation from surface interaction
  prd.attenuation = make_float3( 1.0f, 1.0f, 1.0f );

  // light from a light source or miss program
  prd.radiance = make_float3( 0.0f, 0.0f, 0.0f );

  // next ray to be traced
  prd.origin = make_float3( 0.0f, 0.0f, 0.0f );
  prd.direction = make_float3( 0.0f, 0.0f, 0.0f );

  float3 result = make_float3( 0.0f, 0.0f, 0.0f );

  for(;;) {
      optix::Ray ray(ray_origin, ray_direction, radiance_ray_type, scene_epsilon );
      rtTrace(top_object, ray, prd);

      result += prd.attenuation * prd.radiance;

      if ( prd.done ) {
          break;
      } else if ( prd.depth >= max_depth ) {
        result += prd.attenuation * cutoff_color;
        break;
      }

      prd.depth++;

      // Update ray data for the next path segment
      ray_origin = prd.origin;
      ray_direction = prd.direction;
  }

  float4 acc_val = accum_buffer[launch_index];
  if( frame > 0 ) {
    acc_val = lerp( acc_val, make_float4( result, 0.f ), 1.0f / static_cast<float>( frame+1 ) );
  } else {
    acc_val = make_float4( result, 0.f );
  }
  output_buffer[launch_index] = make_color( make_float3( acc_val ) );
  accum_buffer[launch_index] = acc_val;
}

RT_PROGRAM void exception()
{
  const unsigned int code = rtGetExceptionCode();
  rtPrintf( "Caught exception 0x%X at launch index (%d,%d)\n", code, launch_index.x, launch_index.y );
  output_buffer[launch_index] = make_color( bad_color );
}

