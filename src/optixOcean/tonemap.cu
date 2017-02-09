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

///////////////////////////////////////////////////////////////////////////////

#include "helpers.h"
#include <optix.h>
#include <optix_math.h>


rtDeclareVariable( uint2, launch_index, rtLaunchIndex, );
rtDeclareVariable( uint2, launch_dim,   rtLaunchDim, );

rtDeclareVariable( float, f_exposure, , );

rtBuffer<float4, 2> pre_image;
rtBuffer<uchar4, 2> output_buffer;



RT_PROGRAM void tonemap()
{
  float3 val_Yxy = rgb2Yxy( make_float3( pre_image[ launch_index ] ) );
  
  float Y        = val_Yxy.x; // Y channel is luminance
  float mapped_Y = Y / ( Y + 1.0f );
  
  float3 mapped_Yxy = make_float3( mapped_Y, val_Yxy.y, val_Yxy.z ); 
  float3 mapped_rgb = Yxy2rgb( mapped_Yxy ); 

  output_buffer[ launch_index ] = make_color( mapped_rgb );  
}


