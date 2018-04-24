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

#pragma once

#ifndef LIGHT_DEFINITION_H
#define LIGHT_DEFINITION_H

#include "app_config.h"

#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>

enum LightType
{
  LIGHT_ENVIRONMENT   = 0, // Constant white environment.
  LIGHT_PARALLELOGRAM = 1  // Parallelogram area light.
};

struct LightDefinition
{
  LightType     type; // constant, environment, rectangle (parallelogram)
  // Rectangle lights are defined in world coordinates as footpoint and two vectors spanning a parallelogram.
  // All in world coordinates with no scaling.
  optix::float3 position;
  optix::float3 vecU;
  optix::float3 vecV;
  optix::float3 normal;
  float         area;
  optix::float3 emission;

  // Manual padding to float4 alignment goes here.
  float         unused0;
  float         unused1;
  float         unused2;
};

struct LightSample
{
  optix::float3 position;
  int           index;
  optix::float3 direction;
  float         distance;
  optix::float3 emission;
  float         pdf;
};

#endif // LIGHT_DEFINITION_H
