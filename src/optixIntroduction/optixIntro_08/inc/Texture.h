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

#ifndef TEXTURE_H
#define TEXTURE_H


#include <optix.h>
#include <optixu/optixpp_namespace.h>

#include "inc/Picture.h"

#include <string>
#include <vector>


// Bitfield encoding of the texture channels.
// These are used to remap user format and user data to the internal format.
// Each four bits hold the channel index of red, green, blue, alpha, and luminance. 
// (encoding >> ENC_*_SHIFT) & ENC_MASK gives the channel index if the result is less than 4.
// That encoding allows to automatically swap red and blue, map luminance to RGB (not the other way round though!),
// fill in alpha with input data or force it to one if required.
// 49 remapper functions take care to convert the data types including fixed-point adjustments.

#define ENC_MASK    0xF

#define ENC_RED_SHIFT   0
#define ENC_RED_0       ( 0 << ENC_RED_SHIFT)
#define ENC_RED_1       ( 1 << ENC_RED_SHIFT)
#define ENC_RED_2       ( 2 << ENC_RED_SHIFT)
#define ENC_RED_3       ( 3 << ENC_RED_SHIFT)
#define ENC_RED_NONE    (15 << ENC_RED_SHIFT)

#define ENC_GREEN_SHIFT 4
#define ENC_GREEN_0     ( 0 << ENC_GREEN_SHIFT)
#define ENC_GREEN_1     ( 1 << ENC_GREEN_SHIFT)
#define ENC_GREEN_2     ( 2 << ENC_GREEN_SHIFT)
#define ENC_GREEN_3     ( 3 << ENC_GREEN_SHIFT)
#define ENC_GREEN_NONE  (15 << ENC_GREEN_SHIFT)

#define ENC_BLUE_SHIFT  8
#define ENC_BLUE_0      ( 0 << ENC_BLUE_SHIFT)
#define ENC_BLUE_1      ( 1 << ENC_BLUE_SHIFT)
#define ENC_BLUE_2      ( 2 << ENC_BLUE_SHIFT)
#define ENC_BLUE_3      ( 3 << ENC_BLUE_SHIFT)
#define ENC_BLUE_NONE   (15 << ENC_BLUE_SHIFT)

#define ENC_ALPHA_SHIFT 12
#define ENC_ALPHA_0     ( 0 << ENC_ALPHA_SHIFT)
#define ENC_ALPHA_1     ( 1 << ENC_ALPHA_SHIFT)
#define ENC_ALPHA_2     ( 2 << ENC_ALPHA_SHIFT)
#define ENC_ALPHA_3     ( 3 << ENC_ALPHA_SHIFT)
#define ENC_ALPHA_NONE  (15 << ENC_ALPHA_SHIFT)

#define ENC_LUM_SHIFT   16
#define ENC_LUM_0       ( 0 << ENC_LUM_SHIFT)
#define ENC_LUM_1       ( 1 << ENC_LUM_SHIFT)
#define ENC_LUM_2       ( 2 << ENC_LUM_SHIFT)
#define ENC_LUM_3       ( 3 << ENC_LUM_SHIFT)
#define ENC_LUM_NONE    (15 << ENC_LUM_SHIFT)

#define ENC_CHANNELS_SHIFT 20
#define ENC_CHANNELS_1     (1 << ENC_CHANNELS_SHIFT)
#define ENC_CHANNELS_2     (2 << ENC_CHANNELS_SHIFT)
#define ENC_CHANNELS_3     (3 << ENC_CHANNELS_SHIFT)
#define ENC_CHANNELS_4     (4 << ENC_CHANNELS_SHIFT)

#define ENC_TYPE_SHIFT          24
// These are indices into the remapper table.
#define ENC_TYPE_CHAR           ( 0 << ENC_TYPE_SHIFT)
#define ENC_TYPE_UNSIGNED_CHAR  ( 1 << ENC_TYPE_SHIFT)
#define ENC_TYPE_SHORT          ( 2 << ENC_TYPE_SHIFT)
#define ENC_TYPE_UNSIGNED_SHORT ( 3 << ENC_TYPE_SHIFT)
#define ENC_TYPE_INT            ( 4 << ENC_TYPE_SHIFT)
#define ENC_TYPE_UNSIGNED_INT   ( 5 << ENC_TYPE_SHIFT)
#define ENC_TYPE_FLOAT          ( 6 << ENC_TYPE_SHIFT)
#define ENC_TYPE_UNDEFINED      (15 << ENC_TYPE_SHIFT)

// Flags to indicate that special handling is required.
#define ENC_MISC_SHIFT  28
#define ENC_FIXED_POINT (1 << ENC_MISC_SHIFT)
#define ENC_ALPHA_ONE   (2 << ENC_MISC_SHIFT)

class Texture
{
public:
  Texture();
  ~Texture();
   
  // These would be needed when imoplementing a Texture cache.
  Texture(const Texture &rhs);
  Texture& operator=(const Texture& rhs);

  bool createSampler(optix::Context context,
                     const Picture* picture,
                     bool useSrgb         = false,  // Affects the read mode. Only applied to unsigned byte formats!
                     bool useMipmaps      = false,  // Affects the download of mipmaps. Default is to not download mipmaps.
                     bool useUnnormalized = false); // Affects the texture indexing. Default is normalized 2D coordinates.

  void setWrapMode(RTwrapmode s, RTwrapmode t, RTwrapmode r);

  unsigned int determineHostEncoding(int format, int type) const;
  bool determineDeviceEncoding(int format, int type);
  void convert( void *dst, const void *src, size_t elements, unsigned int hostEncoding ) const;

  optix::TextureSampler getSampler() const;
  int getId() const; // Bindless texture ID.

  unsigned int getWidth() const;
  unsigned int getHeight() const;
  size_t getElementSize() const;

  // Special functions for spherical environment textures.
  void createEnvironment();                       // Creates a small white dummy environment.
  bool createEnvironment(const Picture* picture); // Creates a spherical environment from a previously loaded Picture, using Image face 0 and LOD 0 only.
  bool calculateCDF(optix::Context context); // Create cumulative distribution function importacne sampling of spherical environment lights.
  float getIntegral() const;
  optix::Buffer getBufferCDF_U() const;
  optix::Buffer getBufferCDF_V() const;
  
private:
  unsigned int m_width;
  unsigned int m_height;
  unsigned int m_depth;

  unsigned int m_encoding; // Bitfield describing to which format the texture data should be converted on CUDA side.
  
  RTformat           m_format;
  RTtexturereadmode  m_readMode;
  RTtextureindexmode m_indexMode;

  optix::Buffer         m_buffer; // The format of this buffer defines what kind of texture is behind the sampler!
  optix::TextureSampler m_sampler;

  // These fields are only used for spherical environment maps.
  std::vector<float> m_texels;      // Contains HDR RGBA32F texture data, input to CDF generation.
  float              m_integral;
  optix::Buffer      m_bufferCDF_U;
  optix::Buffer      m_bufferCDF_V;
};

#endif // TEXTURE_H
