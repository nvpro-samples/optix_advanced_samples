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

#include "inc/Texture.h"

#include <IL/il.h>

#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>

#include <algorithm>
#include <cstring>
#include <iostream>

#include "inc/MyAssert.h"


#ifndef M_PI
#define M_PI  3.14159265358979323846264338327950288419716939937510
#endif


Texture::Texture()
: m_width(1) // Make sure getSize2D() works even when there is no texture loaded.
, m_height(1)
, m_depth(1)
, m_encoding(ENC_RED_NONE | ENC_GREEN_NONE | ENC_BLUE_NONE | ENC_ALPHA_NONE | ENC_LUM_NONE)
, m_format(RT_FORMAT_UNSIGNED_BYTE)
, m_readMode(RT_TEXTURE_READ_NORMALIZED_FLOAT)
, m_indexMode(RT_TEXTURE_INDEX_NORMALIZED_COORDINATES)
, m_integral(0.0f)
, m_bufferCDF_U(nullptr)
, m_bufferCDF_V(nullptr)
, m_buffer(nullptr)
, m_sampler(nullptr)
{
}

Texture::~Texture()
{
  // DAR FIXME OptiX objects cannot be destroyed without the context here.
  // Applications which exchange a lot of textures need to track these objects differently.
}

Texture::Texture(const Texture &rhs)
: m_width(rhs.m_width)
, m_height(rhs.m_height)
, m_depth(rhs.m_depth)
, m_encoding(rhs.m_encoding)
, m_format(rhs.m_format)
, m_readMode(rhs.m_readMode)
, m_buffer(rhs.m_buffer)
, m_sampler(rhs.m_sampler)
, m_texels(rhs.m_texels)
, m_integral(rhs.m_integral)
, m_bufferCDF_U(rhs.m_bufferCDF_U)
, m_bufferCDF_V(rhs.m_bufferCDF_V)
{
}
 
Texture& Texture::operator=(const Texture & rhs)
{
  if (&rhs != this)
  {
    m_width       = rhs.m_width;
    m_height      = rhs.m_height;
    m_depth       = rhs.m_depth;
    m_encoding    = rhs.m_encoding;
    m_format      = rhs.m_format;
    m_readMode    = rhs.m_readMode;
    m_indexMode   = rhs.m_indexMode;
    m_buffer      = rhs.m_buffer;
    m_sampler     = rhs.m_sampler;
    m_texels      = rhs.m_texels;
    m_integral    = rhs.m_integral;
    m_bufferCDF_U = rhs.m_bufferCDF_U;
    m_bufferCDF_V = rhs.m_bufferCDF_V;
  }
  return *this;
}

bool Texture::createSampler(optix::Context context,
                            const Picture* picture,
                            bool useSrgb,         // = false // Affects the read mode. Only applied to unsigned byte formats.
                            bool useMipmaps,      // = false // Affects the download of mipmaps. Default is to not download mipmaps.
                            bool useUnnormalized) // = false // Affects the texture indexing. Default is normalized 2D coordinates.
{
  bool success = false;

  if (picture == nullptr)
  {
    std::cerr << "ERROR: createSampler() called with nullptr picture." << std::endl;
    return success;
  }

  // The LOD 0 image of the first face defines the basic settings.
  // This returns nullptr when this image doesn't exist. Everything else in this function relies on it.
  const Image* image = picture->getImageFace(0, 0);

  if (image == nullptr)
  {
    std::cerr << "ERROR: createTextureSamplerAndBuffer() Picture doesn't contain image for LOD 0 of face 0." << std::endl;
    return success;
  }

  unsigned int numFaces = picture->getNumberOfFaces(0); // This is the number of mipmap levels including LOD 0.
  
  const bool isCubemap = picture->isCubemap();

  const unsigned int hostEncoding = determineHostEncoding(image->m_format, image->m_type);

  bool isCreated = false; // Tracks if the TextureSampler and Buffer could be created before filling the buffer.
  try
  {
    // All images in the picture have the same image data format and type (or something is wrong with that picture).
    if (determineDeviceEncoding(image->m_format, image->m_type)) // This sets m_encoding, m_readMode, and m_format;
    {
      m_width  = image->m_width;
      m_height = image->m_height;
      m_depth  = image->m_depth;

      m_sampler = context->createTextureSampler();

      // Set working wrap mode defaults.
      // Cubemaps need RT_WRAP_CLAMP_TO_EDGE to not generate seams at image borders with linear filering.
      // Unnormalized texture indexing cannot use repeating or mirroring wrap modes.
      if (isCubemap || useUnnormalized)
      {
        m_sampler->setWrapMode(0, RT_WRAP_CLAMP_TO_EDGE); 
        m_sampler->setWrapMode(1, RT_WRAP_CLAMP_TO_EDGE);
        m_sampler->setWrapMode(2, RT_WRAP_CLAMP_TO_EDGE); // Set all three modes! OptiX doesn't distinguish among texture targets when checking for compatible wrap modes.
      }
      else
      {
        m_sampler->setWrapMode(0, RT_WRAP_REPEAT);
        m_sampler->setWrapMode(1, RT_WRAP_REPEAT);
        m_sampler->setWrapMode(2, RT_WRAP_REPEAT);
      }

      const RTfiltermode mipmapFilter = (useMipmaps && 1 < numFaces) ? RT_FILTER_LINEAR : RT_FILTER_NONE; // Trilinear or bilinear filtering.
      m_sampler->setFilteringModes(RT_FILTER_LINEAR, RT_FILTER_LINEAR, mipmapFilter);

      // Do not use unnormalized coordinates for cubemaps. // DAR DEBUG Is that even possible?
      m_indexMode = (!isCubemap && useUnnormalized) ? RT_TEXTURE_INDEX_ARRAY_INDEX : RT_TEXTURE_INDEX_NORMALIZED_COORDINATES;
      m_sampler->setIndexingMode(m_indexMode);

      // sRGB to linear conversions only apply to fetches form 8-bit unsigned integer data because the texture hardware does it only for that. 
      // The CUDA manual doesn't mention this. See OpenGL specs for EXT_texture_sRGB_decode. 
      if (useSrgb && image->m_type == IL_UNSIGNED_BYTE)
      {
        if (m_readMode == RT_TEXTURE_READ_ELEMENT_TYPE)
        {
          m_readMode = RT_TEXTURE_READ_ELEMENT_TYPE_SRGB;
        }
        else if (m_readMode == RT_TEXTURE_READ_NORMALIZED_FLOAT)
        {
          m_readMode = RT_TEXTURE_READ_NORMALIZED_FLOAT_SRGB;
        }
      }
      m_sampler->setReadMode(m_readMode);

      m_sampler->setMaxAnisotropy(1.0f); // DAR FIXME Add user control over this parameter.

      if (!isCubemap) // 1D, 2D, or 3D texture.
      {
        // DAR FIXME It's not generally possible to determine the intended texture dimension just by looking at its extents.
        // A 1x1x1 texture could be used with any sampler type: 1D, 2D, or 3D. Potentially breaks the texture access function.
        if (1 < m_depth) // 3D texture
        {
          m_buffer = context->createBuffer(RT_BUFFER_INPUT, m_format, m_width, m_height, m_depth);
        }
        else if (1 < m_height) // 2D Texture
        {
          MY_ASSERT(m_depth == 1);
          m_buffer = context->createBuffer(RT_BUFFER_INPUT, m_format, m_width, m_height);
        }
        else if (1 <= m_width) // 1D Texture.
        {
          MY_ASSERT(m_depth  == 1);
          MY_ASSERT(m_height == 1);
          m_buffer = context->createBuffer(RT_BUFFER_INPUT, m_format, m_width);
        }

        if (useMipmaps && 1 < numFaces)
        {
          m_buffer->setMipLevelCount(numFaces); // Default is 1.
        }

        m_sampler->setBuffer(m_buffer);

        isCreated = true;
      }
      else // cubemap
      {
        if (m_width == m_height && m_depth == 1) // Cubemap images are each square and a single slices.
        {
          // The six cubemap sides are downloaded as six slices in a 3D texture!
          m_buffer = context->createBuffer(RT_BUFFER_INPUT | RT_BUFFER_CUBEMAP, m_format, m_width, m_height, 6);

          if (useMipmaps && 1 < numFaces)
          {
            m_buffer->setMipLevelCount(numFaces); // Default is 1.
          }

          m_sampler->setBuffer(m_buffer);

          isCreated = true;
        }
      }
    }

    if (!isCreated)
    {
      std::cerr << "ERROR: createSampler() Could not create TextureSampler or Buffer" << std::endl;
      return success;
    }

    // If the TextureSampler and Buffer could be created, fill the buffer with the pixel data.

    if (!isCubemap) // 1D, 2D 3D with optional mipmaps. No layered texture support in this routine!
    {
      unsigned int numFaces = picture->getNumberOfFaces(0); // This is the number of mipmap levels including LOD 0.

      for (unsigned int indexFace = 0; indexFace < numFaces && (indexFace == 0 || useMipmaps); ++indexFace)
      {
        const Image* image = picture->getImageFace(0, indexFace);

        if (image != nullptr)
        {
          void *dst = m_buffer->map(indexFace, RT_BUFFER_MAP_WRITE_DISCARD);
          convert(dst, image->m_pixels, image->m_width * image->m_height * image->m_depth, hostEncoding);
          m_buffer->unmap(indexFace);
        }
      }
    }
    else // if (isCubemap)
    {
      unsigned int numImages = picture->getNumberOfImages(); // These are the six sides of the cubemap.
      MY_ASSERT(numImages == 6);

      for (unsigned int indexImage = 0; indexImage < numImages; ++indexImage)
      {
        unsigned int numFaces = picture->getNumberOfFaces(indexImage); // This is the number of mipmap levels including LOD 0 of this cubemap image (6).

        for (unsigned int indexFace = 0; indexFace < numFaces && (indexFace == 0 || useMipmaps); ++indexFace)
        {
          const Image* image = picture->getImageFace(indexImage, indexFace);

          if (image != nullptr )
          {
            unsigned char* dst = static_cast<unsigned char*>(m_buffer->map(indexFace, RT_BUFFER_MAP_WRITE_DISCARD));

            // Calculate the proper offset of this cubemap side's mipmap level.
            dst += indexImage * image->m_width * image->m_height * getElementSize();

            convert(dst, image->m_pixels, image->m_width * image->m_height, hostEncoding); // 2D!
            m_buffer->unmap(indexFace);
          }
        }
      }
    }
    success = true;
  }
  catch(optix::Exception& e)
  {
    std::cerr << e.getErrorString() << std::endl;
  }
  return success;
}


// Use with standard texture sampler declarations.
optix::TextureSampler Texture::getSampler() const
{
  return m_sampler;
}

// Use with bindless textures. (Hardware support on Kepler and higher!)
int Texture::getId() const
{
  return (m_sampler) ? m_sampler->getId() : RT_TEXTURE_ID_NULL;
}

void Texture::setWrapMode(RTwrapmode s, RTwrapmode t, RTwrapmode r)
{
  m_sampler->setWrapMode(0, s);
  m_sampler->setWrapMode(1, t);
  m_sampler->setWrapMode(2, r);
}

unsigned int Texture::getWidth() const
{
  return m_width;
}

unsigned int Texture::getHeight() const
{
  return m_height;
}

// For OpenGL interop these formats are supported by CUDA according to the current manual on cudaGraphicsGLRegisterImage:
// GL_RED, GL_RG, GL_RGBA, GL_LUMINANCE, GL_ALPHA, GL_LUMINANCE_ALPHA, GL_INTENSITY
// {GL_R, GL_RG, GL_RGBA} x {8, 16, 16F, 32F, 8UI, 16UI, 32UI, 8I, 16I, 32I}
// {GL_LUMINANCE, GL_ALPHA, GL_LUMINANCE_ALPHA, GL_INTENSITY} x {8, 16, 16F_ARB, 32F_ARB, 8UI_EXT, 16UI_EXT, 32UI_EXT, 8I_EXT, 16I_EXT, 32I_EXT}

// The following mapping is done for host textures. RGB formats will be expanded to RGBA.

// DAR While single and dual channel textures can easily be uploaded, the texture doesn't know what the destination format actually is,
// that is, a LUMINANCE_ALPHA texture returns the luminance in the red channel and the alpha in the green channel.
// That doesn't work the same way as OpenGL which copies luminance to all three RGB channels automatically.
// DAR HACK  Instead of tailoring the texture lookup functions in the device code, expand all textures to four channels.
// DAR FIXME Reconsider storing 1, 2, and 4 channel textures to save memory.
bool Texture::determineDeviceEncoding(int format, int type)
{
  bool success = true;

  switch (format)
  {
    case IL_RGB:
    case IL_BGR:
      m_encoding = ENC_RED_0 | ENC_GREEN_1 | ENC_BLUE_2 | ENC_ALPHA_3 | ENC_LUM_NONE | ENC_CHANNELS_4 | ENC_ALPHA_ONE; // (R, G, B, 1)
      break;
    case IL_RGBA:
    case IL_BGRA:
      m_encoding = ENC_RED_0 | ENC_GREEN_1 | ENC_BLUE_2 | ENC_ALPHA_3 | ENC_LUM_NONE | ENC_CHANNELS_4; // (R, G, B, A)
      break;
    case IL_LUMINANCE: 
      // m_encoding = ENC_RED_NONE | ENC_GREEN_NONE | ENC_BLUE_NONE | ENC_ALPHA_NONE | ENC_LUM_0 | ENC_CHANNELS_1; 
      m_encoding = ENC_RED_0 | ENC_GREEN_1 | ENC_BLUE_2 | ENC_ALPHA_3 | ENC_LUM_NONE | ENC_CHANNELS_4 | ENC_ALPHA_ONE; // Expands to (L, L, L, 1)
      break;
    case IL_ALPHA:
      // m_encoding = ENC_RED_NONE | ENC_GREEN_NONE | ENC_BLUE_NONE | ENC_ALPHA_0 | ENC_LUM_NONE | ENC_CHANNELS_1;
      m_encoding = ENC_RED_0 | ENC_GREEN_1 | ENC_BLUE_2 | ENC_ALPHA_3 | ENC_LUM_NONE | ENC_CHANNELS_4; // Expands to (0, 0, 0, A)
      break;
    case IL_LUMINANCE_ALPHA:
      // m_encoding = ENC_RED_NONE | ENC_GREEN_NONE | ENC_BLUE_NONE | ENC_ALPHA_1 | ENC_LUM_0 | ENC_CHANNELS_2;
      m_encoding = ENC_RED_0 | ENC_GREEN_1 | ENC_BLUE_2 | ENC_ALPHA_3 | ENC_LUM_NONE| ENC_CHANNELS_4; // Expands to (L, L, L, A)
      break;
    default:
      MY_ASSERT(!"Unsupported user pixel format.");
      m_encoding = ENC_RED_NONE | ENC_GREEN_NONE | ENC_BLUE_NONE | ENC_ALPHA_NONE | ENC_LUM_NONE; // DAR No channels here would break the RTformat calculation below.
      success = false;
      break;
  }

  switch (type)
  {
    case IL_UNSIGNED_BYTE:
      m_encoding |= ENC_TYPE_UNSIGNED_CHAR | ENC_FIXED_POINT;
      m_readMode = RT_TEXTURE_READ_NORMALIZED_FLOAT;
      m_format = RTformat(((unsigned int)(RT_FORMAT_UNSIGNED_BYTE) + ((m_encoding >> ENC_CHANNELS_SHIFT) & 0xF) - 1));
      break;
    case IL_UNSIGNED_SHORT:
      m_encoding |= ENC_TYPE_UNSIGNED_SHORT | ENC_FIXED_POINT;
      m_readMode = RT_TEXTURE_READ_NORMALIZED_FLOAT;
      m_format = RTformat(((unsigned int)(RT_FORMAT_UNSIGNED_SHORT) + ((m_encoding >> ENC_CHANNELS_SHIFT) & 0xF) - 1));
      break;
    case IL_UNSIGNED_INT:
      m_encoding |= ENC_TYPE_UNSIGNED_INT | ENC_FIXED_POINT;
      m_readMode = RT_TEXTURE_READ_NORMALIZED_FLOAT;
      m_format = RTformat(((unsigned int)(RT_FORMAT_UNSIGNED_INT) + ((m_encoding >> ENC_CHANNELS_SHIFT) & 0xF) - 1));
      break;
    case IL_BYTE:
      m_encoding |= ENC_TYPE_CHAR | ENC_FIXED_POINT;
      m_readMode = RT_TEXTURE_READ_NORMALIZED_FLOAT;
      m_format = RTformat(((unsigned int)(RT_FORMAT_BYTE) + ((m_encoding >> ENC_CHANNELS_SHIFT) & 0xF) - 1));
      break;
    case IL_SHORT:
      m_encoding |= ENC_TYPE_SHORT | ENC_FIXED_POINT;
      m_readMode = RT_TEXTURE_READ_NORMALIZED_FLOAT;
      m_format = RTformat(((unsigned int)(RT_FORMAT_SHORT) + ((m_encoding >> ENC_CHANNELS_SHIFT) & 0xF) - 1));
      break;
    case IL_INT:
      m_encoding |= ENC_TYPE_INT | ENC_FIXED_POINT;
      m_readMode = RT_TEXTURE_READ_NORMALIZED_FLOAT;
      m_format = RTformat(((unsigned int)(RT_FORMAT_INT) + ((m_encoding >> ENC_CHANNELS_SHIFT) & 0xF) - 1));
      break;
    case IL_FLOAT:
      m_encoding |= ENC_TYPE_FLOAT;
      m_readMode = RT_TEXTURE_READ_ELEMENT_TYPE;
      m_format = RTformat(((unsigned int)(RT_FORMAT_FLOAT) + ((m_encoding >> ENC_CHANNELS_SHIFT) & 0xF) - 1));
      break;
    default:
      MY_ASSERT(!"Unsupported user data format.");
      success = false;
      break;
  }

  return success;
}

// The ENC_RED|GREEN|BLUE|ALPHA|LUM codes define from which source channel is read when writing R, G, B, A or L.
unsigned int Texture::determineHostEncoding(int format, int type) const
{
  unsigned int encoding;

  switch (format)
  {
    case IL_RGB:
      encoding = ENC_RED_0 | ENC_GREEN_1 | ENC_BLUE_2 | ENC_ALPHA_NONE | ENC_LUM_NONE | ENC_CHANNELS_3;
      break;
    case IL_RGBA:
      encoding = ENC_RED_0 | ENC_GREEN_1 | ENC_BLUE_2 | ENC_ALPHA_3 | ENC_LUM_NONE | ENC_CHANNELS_4;
      break;
    case IL_BGR:
      encoding = ENC_RED_2 | ENC_GREEN_1 | ENC_BLUE_0 | ENC_ALPHA_NONE | ENC_LUM_NONE | ENC_CHANNELS_3;
      break;
    case IL_BGRA:
      encoding = ENC_RED_2 | ENC_GREEN_1 | ENC_BLUE_0 | ENC_ALPHA_3 | ENC_LUM_NONE | ENC_CHANNELS_4;
      break;
    case IL_LUMINANCE: 
      encoding = ENC_RED_0 | ENC_GREEN_0 | ENC_BLUE_0 | ENC_ALPHA_NONE | ENC_LUM_NONE | ENC_CHANNELS_1; // Copy luminance in channel 0 to RGB.
      break;
    case IL_ALPHA:
      encoding = ENC_RED_NONE | ENC_GREEN_NONE | ENC_BLUE_NONE | ENC_ALPHA_0 | ENC_LUM_NONE | ENC_CHANNELS_1;
      break;
    case IL_LUMINANCE_ALPHA:
      encoding = ENC_RED_0 | ENC_GREEN_0 | ENC_BLUE_0 | ENC_ALPHA_1 | ENC_LUM_NONE | ENC_CHANNELS_2; // Copy luminance in channel 0 to RGB.
      break;
    default:
      MY_ASSERT(!"Unsupported user pixel format.");
      encoding = ENC_RED_NONE | ENC_GREEN_NONE | ENC_BLUE_NONE | ENC_ALPHA_NONE | ENC_LUM_NONE;
      break;
  }

  switch (type)
  {
    case IL_UNSIGNED_BYTE:
      encoding |= ENC_TYPE_UNSIGNED_CHAR;
      break;
    case IL_UNSIGNED_SHORT:
      encoding |= ENC_TYPE_UNSIGNED_SHORT;
      break;
    case IL_UNSIGNED_INT:
      encoding |= ENC_TYPE_UNSIGNED_INT;
      break;
    case IL_BYTE:
      encoding |= ENC_TYPE_CHAR;
      break;
    case IL_SHORT:
      encoding |= ENC_TYPE_SHORT;
      break;
    case IL_INT:
      encoding |= ENC_TYPE_INT;
      break;
    case IL_FLOAT:
      encoding |= ENC_TYPE_FLOAT;
      break;
    default:
      MY_ASSERT(!"Unsupported user data format.");
      break;
  }
        
  return encoding;
}


size_t Texture::getElementSize() const
{
  switch (m_format)
  {
  case RT_FORMAT_FLOAT:
    return sizeof(float);
  case RT_FORMAT_FLOAT2:
    return sizeof(float) * 2;
  case RT_FORMAT_FLOAT3:
    return sizeof(float) * 3;
  case RT_FORMAT_FLOAT4:
    return sizeof(float) * 4;
  case RT_FORMAT_BYTE:
    return sizeof(char);
  case RT_FORMAT_BYTE2:
    return sizeof(char) * 2;
  case RT_FORMAT_BYTE3:
    return sizeof(char) * 3;
  case RT_FORMAT_BYTE4:
    return sizeof(char) * 4;
  case RT_FORMAT_UNSIGNED_BYTE:
    return sizeof(unsigned char);
  case RT_FORMAT_UNSIGNED_BYTE2:
    return sizeof(unsigned char) * 2;
  case RT_FORMAT_UNSIGNED_BYTE3:
    return sizeof(unsigned char) * 3;
  case RT_FORMAT_UNSIGNED_BYTE4:
    return sizeof(unsigned char) * 4;
  case RT_FORMAT_SHORT:
    return sizeof(short);
  case RT_FORMAT_SHORT2:
    return sizeof(short) * 2;
  case RT_FORMAT_SHORT3:
    return sizeof(short) * 3;
  case RT_FORMAT_SHORT4:
    return sizeof(short) * 4;
  case RT_FORMAT_UNSIGNED_SHORT:
    return sizeof(unsigned short);
  case RT_FORMAT_UNSIGNED_SHORT2:
    return sizeof(unsigned short) * 2;
  case RT_FORMAT_UNSIGNED_SHORT3:
    return sizeof(unsigned short) * 3;
  case RT_FORMAT_UNSIGNED_SHORT4:
    return sizeof(unsigned short) * 4;
  case RT_FORMAT_INT:
    return sizeof(int);
  case RT_FORMAT_INT2:
    return sizeof(int) * 2;
  case RT_FORMAT_INT3:
    return sizeof(int) * 3;
  case RT_FORMAT_INT4:
    return sizeof(int) * 4;
  case RT_FORMAT_UNSIGNED_INT:
    return sizeof(unsigned int);
  case RT_FORMAT_UNSIGNED_INT2:
    return sizeof(unsigned int) * 2;
  case RT_FORMAT_UNSIGNED_INT3:
    return sizeof(unsigned int) * 3;
  case RT_FORMAT_UNSIGNED_INT4:
    return sizeof(unsigned int) * 4;
  case RT_FORMAT_UNKNOWN:
  case RT_FORMAT_USER:
  default:
    MY_ASSERT(!"Unknown element size! (unknown or user format)");
    return 0;
  }
}


template<typename T> 
T getAlphaOne()
{
  return (std::numeric_limits<T>::is_integer ? std::numeric_limits<T>::max() : T(1));
}

// Fixed point adjustment for integer data D and S.
template<typename D, typename S>
D adjust(S value)
{
  int dstBits = int(sizeof(D)) * 8;
  int srcBits = int(sizeof(S)) * 8;

  D result = D(0); // Clear bits to allow OR operations.

  if (std::numeric_limits<D>::is_signed)
  {
    if (std::numeric_limits<S>::is_signed)
    {
      // D signed, S signed
      if (dstBits <= srcBits)
      {
        // More bits provided than needed. Use the most significant bits of value.
        result = D(value >> (srcBits - dstBits));
      }
      else
      {
        // Shift value into the most significant bits of result and replicate value into the lower bits until all are touched.
        int shifts = dstBits - srcBits;
        result = D(value << shifts);            // This sets the destination sign bit as well.
        value &= std::numeric_limits<S>::max(); // Clear the sign bit inside the source value.
        srcBits--;                              // Reduce the number of srcBits used to replicate the remaining data.
        shifts -= srcBits;                      // Subtracting the now one smaller srcBits from shifts means the next shift will fill up with the remaining non-sign bits as intended.
        while (0 <= shifts)
        {
          result |= D(value << shifts);
          shifts -= srcBits;
        }
        if (shifts < 0) // There can be one to three empty bits left blank in the result now.
        {
          result |= D(value >> -shifts); // Shift to the right to get the most significant bits of value into the least significant destination bits.
        }
      }
    }
    else
    {
      // D signed, S unsigned
      if (dstBits <= srcBits)
      {
        // More bits provided than needed. Use the most significant bits of value.
        result = D(value >> (srcBits - dstBits + 1)); // + 1 because the destination is signed and the value needs to remain positive.
      }
      else
      {
        // Shift value into the most significant bits of result, keep the sign clear, and replicate value into the lower bits until all are touched.
        int shifts = dstBits - srcBits - 1; // - 1 because the destination is signed and the value needs to remain positive.
        while (0 <= shifts)
        {
          result |= D(value << shifts);
          shifts -= srcBits;
        }
        if (shifts < 0)
        {
          result |= D(value >> -shifts);
        }
      }
    }
  }
  else
  {
    if (std::numeric_limits<S>::is_signed)
    {
      // D unsigned, S signed
      value = std::max(S(0), value); // Only the positive values will be transferred.
      srcBits--;                     // Skip the sign bit. Means equal bit size won't happen here.
      if (dstBits <= srcBits)        // When it's really bigger it has at least 7 bits more, no need to care for dangling bits
      {
        result = D(value >> (srcBits - dstBits));
      }
      else
      {
        int shifts = dstBits - srcBits;
        while (0 <= shifts)
        {
          result |= D(value << shifts);
          shifts -= srcBits;
        }
        if (shifts < 0)
        {
          result |= D(value >> -shifts);
        }
      }
    }
    else
    {
      // D unsigned, S unsigned
      if (dstBits <= srcBits)
      {
        // More bits provided than needed. Use the most significant bits of value.
        result = D(value >> (srcBits - dstBits));
      }
      else
      {
        // Shift value into the most significant bits of result and replicate into the lower ones until all bits are touched.
        int shifts = dstBits - srcBits;
        while (0 <= shifts) 
        {
          result |= D(value << shifts);
          shifts -= srcBits;
        } 
        // Both bit sizes are even multiples of 8, there are no trailing bits here.
        MY_ASSERT(shifts == -srcBits);
      }
    }
  }
  return result;
}


template<typename D, typename S>
void remapAdjust(void *dst, const void *src, size_t count, unsigned int dstEncoding, unsigned int srcEncoding)
{
  const S *psrc = reinterpret_cast<const S *>(src);
  D *pdst = reinterpret_cast<D *>(dst);
  unsigned int dstChannels = (dstEncoding >> ENC_CHANNELS_SHIFT) & ENC_MASK;
  unsigned int srcChannels = (srcEncoding >> ENC_CHANNELS_SHIFT) & ENC_MASK;
  bool fixedPoint = !!(dstEncoding & ENC_FIXED_POINT);
  bool alphaOne   = !!(dstEncoding & ENC_ALPHA_ONE);

  while (count--)
  {
    unsigned int shift = ENC_RED_SHIFT;
    for (unsigned int i = 0; i < 5; ++i, shift += 4) // Five possible channels: R, G, B, A, L
    {
      unsigned int d = (dstEncoding >> shift) & ENC_MASK;
      if (d < 4) // This data channel exists inside the destination.
      {
        unsigned int s = (srcEncoding >> shift) & ENC_MASK;
        // If destination alpha was added to support this format or if no source data is given for alpha, fill it with 1.
        if (shift == ENC_ALPHA_SHIFT && (alphaOne || 4 <= s))
        {
          pdst[d] = getAlphaOne<D>();
        }
        else
        {
          if (s < 4) // There is data for this channel inside the source. (This could be a luminance to RGB mapping as well).
          {
            S value = psrc[s];
            pdst[d] = (fixedPoint) ? adjust<D>(value) : D(value);
          }
          else // no value provided
          {
            pdst[d] = D(0);
          }
        }
      }
    }
    pdst += dstChannels;
    psrc += srcChannels;
  }
}

// Straight channel copy with no adjustment. Since the data types match, fixed point doesn't matter.
template<typename T>
void remapCopy(void *dst, const void *src, size_t count, unsigned int dstEncoding, unsigned int srcEncoding)
{
  const T *psrc = reinterpret_cast<const T *>(src);
  T *pdst = reinterpret_cast<T *>(dst);
  unsigned int dstChannels = (dstEncoding >> ENC_CHANNELS_SHIFT) & ENC_MASK;
  unsigned int srcChannels = (srcEncoding >> ENC_CHANNELS_SHIFT) & ENC_MASK;
  bool alphaOne = !!(dstEncoding & ENC_ALPHA_ONE);

  while (count--)
  {
    unsigned int shift = ENC_RED_SHIFT;
    for (unsigned int i = 0; i < 5; ++i, shift += 4) // Five possible channels: R, G, B, A, L
    {
      unsigned int d = (dstEncoding >> shift) & ENC_MASK;
      if (d < 4) // This data channel exists inside the destination.
      {
        unsigned int s = (srcEncoding >> shift) & ENC_MASK;
        if (shift == ENC_ALPHA_SHIFT && (alphaOne || 4 <= s))
        {
          pdst[d] = getAlphaOne<T>();
        }
        else
        {
          pdst[d] = (s < 4) ? psrc[s] : T(0);
        }
      }
    }
    pdst += dstChannels;
    psrc += srcChannels;
  }
}

template<typename D>
void remapFromFloat(void *dst, const void *src, size_t count, unsigned int dstEncoding, unsigned int srcEncoding)
{
  const float *psrc = reinterpret_cast<const float *>(src);
  D *pdst = reinterpret_cast<D *>(dst);
  unsigned int dstChannels = (dstEncoding >> ENC_CHANNELS_SHIFT) & ENC_MASK;
  unsigned int srcChannels = (srcEncoding >> ENC_CHANNELS_SHIFT) & ENC_MASK;
  bool fixedPoint = !!(dstEncoding & ENC_FIXED_POINT);
  bool alphaOne   = !!(dstEncoding & ENC_ALPHA_ONE);

  while (count--)
  {
    unsigned int shift = ENC_RED_SHIFT;
    for (unsigned int i = 0; i < 5; ++i, shift += 4)
    {
      unsigned int d = (dstEncoding >> shift) & ENC_MASK;
      if (d < 4) // This data channel exists inside the destination.
      {
        unsigned int s = (srcEncoding >> shift) & ENC_MASK;
        if (shift == ENC_ALPHA_SHIFT && (alphaOne || 4 <= s))
        {
          pdst[d] = getAlphaOne<D>();
        }
        else
        {
          if (s < 4) // This data channel exists inside the source.
          {
            float value = psrc[s];
            if (fixedPoint)
            {
              MY_ASSERT(std::numeric_limits<D>::is_integer); // Destination with float format cannot be fixed point.

              float minimum = (std::numeric_limits<D>::is_signed) ? -1.0f : 0.0f;
              value = std::min(std::max(minimum, value), 1.0f);
              pdst[d] = D(std::numeric_limits<D>::max() * value); // Scaled copy.
            }
            else // element type, clamped copy.
            {
              float maximum = float(std::numeric_limits<D>::max()); // This will run out of precision for int and unsigned int.
              float minimum = -maximum;
              pdst[d] = D(std::min(std::max(minimum, value), maximum));
            }
          }
          else // no value provided
          {
            pdst[d] = D(0);
          }
        }
      }
    }
    pdst += dstChannels;
    psrc += srcChannels;
  }
}

template<typename S>
void remapToFloat(void *dst, const void *src, size_t count, unsigned int dstEncoding, unsigned int srcEncoding)
{
  const S *psrc = reinterpret_cast<const S *>(src);
  float *pdst = reinterpret_cast<float *>(dst);
  unsigned int dstChannels = (dstEncoding >> ENC_CHANNELS_SHIFT) & ENC_MASK;
  unsigned int srcChannels = (srcEncoding >> ENC_CHANNELS_SHIFT) & ENC_MASK;
  bool alphaOne = !!(dstEncoding & ENC_ALPHA_ONE);

  while (count--)
  {
    unsigned int shift = ENC_RED_SHIFT;
    for (unsigned int i = 0; i < 5; ++i, shift += 4)
    {
      unsigned int d = (dstEncoding >> shift) & ENC_MASK;
      if (d < 4) // This data channel exists inside the destination.
      {
        unsigned int s = (srcEncoding >> shift) & ENC_MASK;
        if (shift == ENC_ALPHA_SHIFT && (alphaOne || 4 <= s))
        {
          pdst[d] = 1.0f;
        }
        else
        {
          // If there is data for this channel just cast it straight in.
          // This will run out of precision for int and unsigned int source data.
          pdst[d] = (s < 4) ? float(psrc[s]) : 0.0f;
        }
      }
    }
    pdst += dstChannels;
    psrc += srcChannels;
  }
}


typedef void (*PFNREMAP)(void *dst, const void *src, size_t count, unsigned int dstEncoding, unsigned int srcEncoding);

// Function table with 49 texture format conversion routines from loaded image data to supported CUDA texture formats.
// Index is [destination type][source type]
PFNREMAP remappers[7][7] = 
{
  {
    remapCopy<char>,
    remapAdjust<char, unsigned char>,
    remapAdjust<char, short>,
    remapAdjust<char, unsigned short>,
    remapAdjust<char, int>,
    remapAdjust<char, unsigned int>,
    remapFromFloat<char>
  },
  { 
    remapAdjust<unsigned char, char>,
    remapCopy<unsigned char>,
    remapAdjust<unsigned char, short>,
    remapAdjust<unsigned char, unsigned short>,
    remapAdjust<unsigned char, int>,
    remapAdjust<unsigned char, unsigned int>,
    remapFromFloat<unsigned char>
  },
  { 
    remapAdjust<short, char>,
    remapAdjust<short, unsigned char>,
    remapCopy<short>,
    remapAdjust<short, unsigned short>,
    remapAdjust<short, int>,
    remapAdjust<short, unsigned int>,
    remapFromFloat<short>
  },
  {
    remapAdjust<unsigned short, char>,
    remapAdjust<unsigned short, unsigned char>,
    remapAdjust<unsigned short, short>,
    remapCopy<unsigned short>,
    remapAdjust<unsigned short, int>,
    remapAdjust<unsigned short, unsigned int>,
    remapFromFloat<unsigned short>
  },
  { 
    remapAdjust<int, char>,
    remapAdjust<int, unsigned char>,
    remapAdjust<int, short>,
    remapAdjust<int, unsigned short>,
    remapCopy<int>,
    remapAdjust<int, unsigned int>,
    remapFromFloat<int>
  },
  {
    remapAdjust<unsigned int, char>,
    remapAdjust<unsigned int, unsigned char>,
    remapAdjust<unsigned int, short>,
    remapAdjust<unsigned int, unsigned short>,
    remapAdjust<unsigned int, int>,
    remapCopy<unsigned int>,
    remapFromFloat<unsigned int>
  },
  {
    remapToFloat<char>,
    remapToFloat<unsigned char>,
    remapToFloat<short>,
    remapToFloat<unsigned short>,
    remapToFloat<int>,
    remapToFloat<unsigned int>,
    remapCopy<float>
  }
};


// Finally the function which converts any loaded image into a texture format supported by CUDA (1, 2, 4 channels only).
void Texture::convert(void *dst, const void *src, size_t elements, unsigned int hostEncoding) const
{
  // Only destination encoding knows about the fixed-point encoding. For straight data memcpy() cases that is irrelevant.
  if ((m_encoding & ~ENC_FIXED_POINT) == hostEncoding)
  {
    memcpy(dst, src, elements * getElementSize()); // The fastest path.
  }
  else
  {
    unsigned int dstType = (m_encoding   >> ENC_TYPE_SHIFT) & ENC_MASK;
    unsigned int srcType = (hostEncoding >> ENC_TYPE_SHIFT) & ENC_MASK;
    MY_ASSERT(dstType < 7 && srcType < 7); 
          
    PFNREMAP pfn = remappers[dstType][srcType];

    (*pfn)(dst, src, elements, m_encoding, hostEncoding);
  }
}

// The following functions are used to build the data needed for an importance sampled spherical HDR environment map. 
// DAR FIXME Put this into a separate class derived from Texture.

bool Texture::createEnvironment(const Picture* picture)
{
  if (picture == nullptr)
  {
    std::cerr << "ERROR: Environment picture is nullptr! Creating white dummy environment." << std::endl;
    createEnvironment();
    return false;
  }

  // Get the Image pointer at image 0 and face 0. Returns nullptr if that doesn't exist.
  const Image* image = picture->getImageFace(0, 0);

  if (image == nullptr)
  {
    std::cerr << "ERROR: getImage(0, 0) returned nullptr! Creating white dummy environment." << std::endl;
    createEnvironment();
    return false;
  }

  // If there is any data in that 2D image create the texture.
  if (0 < image->m_nob && image->m_depth == 1)
  {
    m_width  = image->m_width;
    m_height = image->m_height;
    m_depth  = image->m_depth;

    unsigned int hostEncoding = determineHostEncoding(image->m_format, image->m_type);

    // Convert the input image to RGBA32F.
    // Note, this function only handles HDR environment maps (*.hdr and *.exr files).
    // Fixed point LDR textures will remain unnormalized. E.g. unsigned byte 255 will be converted to float 255.0f. 
    // (Just because there is no suitable conversion routine implemented for that.)
    m_encoding  = ENC_RED_0 | ENC_GREEN_1 | ENC_BLUE_2 | ENC_ALPHA_3 | ENC_LUM_NONE | ENC_CHANNELS_4 | ENC_ALPHA_ONE | ENC_TYPE_FLOAT;
    
    m_format    = RT_FORMAT_FLOAT4; // == RTformat(((unsigned int)(RT_FORMAT_FLOAT) + ((m_encoding >> ENC_CHANNELS_SHIFT) & 0xF) - 1));
    m_readMode  = RT_TEXTURE_READ_ELEMENT_TYPE;
    m_indexMode = RT_TEXTURE_INDEX_NORMALIZED_COORDINATES;

    // Converting into a local memory for the CDF generation routines to use the expected RGBA32F data.
    m_texels.resize(image->m_width * image->m_height * 4);
    convert(m_texels.data(), image->m_pixels, image->m_width * image->m_height, hostEncoding); // After this m_texels contains RGBA32F data.
  }
  return true;
}

// When not providing a pointer to a Picture, create dummy image data to fill the environment map sampler 
// and CDF variables when another miss shader is used.
// That allows to switch miss shader implementations without recompilation of the application.
void Texture::createEnvironment()
{
  m_width  = 8;
  m_height = 4;
  m_depth  = 1;
 
  m_texels.resize(m_width * m_height * 4);
  
  float* rgba = m_texels.data();

  // Debug diffuse scattering with a uniform environment. 
  // (White sphere in white room must be white!)
  for (unsigned int y = 0; y < m_height; ++y)
  {
    for (unsigned int x = 0; x < m_width; ++x)
    {
      unsigned int i = (y * m_width + x) * 4;
      rgba[i    ] = 1.0f;
      rgba[i + 1] = 1.0f;
      rgba[i + 2] = 1.0f;
      rgba[i + 3] = 1.0f;
    }
  }
}

// Implement a simple Gaussian 3x3 filter with sigma = 0.5
// Needed for the CDF generation of the importance sampled HDR environment texture light.
static float gaussianFilter(const float* rgba, unsigned int width, unsigned int height, unsigned int x, unsigned int y)
{
  // Lookup is repeated in x and clamped to edge in y.
  unsigned int left   = (0 < x)          ? x - 1 : width - 1; // repeat
  unsigned int right  = (x < width - 1)  ? x + 1 : 0;         // repeat
  unsigned int bottom = (0 < y)          ? y - 1 : y;         // clamp
  unsigned int top    = (y < height - 1) ? y + 1 : y;         // clamp
  
  // Center
  const float *p = rgba + (width * y + x) * 4;
  float intensity = (p[0] + p[1] + p[2]) * 0.619347f;

  // 4-neighbours
  p = rgba + (width * bottom + x) * 4;
  float f = p[0] + p[1] + p[2];
  p = rgba + (width * y + left) * 4;
  f += p[0] + p[1] + p[2];
  p = rgba + (width * y + right) * 4;
  f += p[0] + p[1] + p[2];
  p = rgba + (width * top + x) * 4;
  f += p[0] + p[1] + p[2];
  intensity += f * 0.0838195f;

  // 8-neighbours corners
  p = rgba + (width * bottom + left) * 4;
  f  = p[0] + p[1] + p[2];
  p = rgba + (width * bottom + right) * 4;
  f += p[0] + p[1] + p[2];
  p = rgba + (width * top + left) * 4;
  f += p[0] + p[1] + p[2];
  p = rgba + (width * top + right) * 4;
  f += p[0] + p[1] + p[2];
  intensity += f * 0.0113437f;

  return intensity / 3.0f;
}
 
// Create cumulative distribution function for importance sampling of spherical environment lights.
// This is a textbook implementation for the CDF generation of a spherical HDR environment.
// See "Physically Based Rendering" v2, chapter 14.6.5 on Infinite Area Lights.
bool Texture::calculateCDF(optix::Context context)
{
  if (m_texels.empty() || (m_texels.size() != m_width * m_height * 4))
  {
    return false;
  }

  const float *rgba = m_texels.data();

  // The original data needs to be retained to calculate the PDF.
  float *funcU = new float[m_width * m_height];
  float *funcV = new float[m_height + 1];

  float sum = 0.0f;
  // First generate the function data.
  for (unsigned int y = 0; y < m_height; ++y)
  {
    // Scale distibution by the sine to get the sampling uniform. (Avoid sampling more values near the poles.)
    // See Physically Based Rendering v2, chapter 14.6.5 on Infinite Area Lights, page 728.
    float sinTheta = float(sin(M_PI * (double(y) + 0.5) / double(m_height))); // Make this as accurate as possible.

    for (unsigned int x = 0; x < m_width; ++x)
    {
      // Filter to keep the piecewise linear function intact for samples with zero value next to non-zero values.
      const float value = gaussianFilter(rgba, m_width, m_height, x, y);
      funcU[y * m_width + x] = value * sinTheta;

      // Compute integral over the actual function.
      const float *p = rgba + (y * m_width + x) * 4;
      const float intensity = (p[0] + p[1] + p[2]) / 3.0f;
      sum += intensity * sinTheta;
    }
  }

  // This integral is used inside the light sampling function (see sysEnvironmentIntegral).
  m_integral = sum * 2.0f * M_PIf * M_PIf / float(m_width * m_height);

  // Now generate the CDF data.
  // Normalized 1D distributions in the rows of the 2D buffer, and the marginal CDF in the 1D buffer.
  // Include the starting 0.0f and the ending 1.0f to avoid special cases during the continuous sampling.
  float *cdfU = new float[(m_width + 1) * m_height];
  float *cdfV = new float[m_height + 1];

  for (unsigned int y = 0; y < m_height; ++y)
  {
    unsigned int row = y * (m_width + 1); // Watch the stride!
    cdfU[row + 0] = 0.0f; // CDF starts at 0.0f.

    for (unsigned int x = 1; x <= m_width; ++x)
    {
      unsigned int i = row + x;
      cdfU[i] = cdfU[i - 1] + funcU[y * m_width + x - 1]; // Attention, funcU is only m_width wide! 
    }

    const float integral = cdfU[row + m_width]; // The integral over this row is in the last element.
    funcV[y] = integral;                        // Store this as function values of the marginal CDF.

    if (integral != 0.0f)
    {
      for (unsigned int x = 1; x <= m_width; ++x)
      {
        cdfU[row + x] /= integral;
      }
    }
    else // All texels were black in this row. Generate an equal distribution.
    {
      for (unsigned int x = 1; x <= m_width; ++x)
      {
        cdfU[row + x] = float(x) / float(m_width);
      }
    }
  }

  // Now do the same thing with the marginal CDF.
  cdfV[0] = 0.0f; // CDF starts at 0.0f.
  for (unsigned int y = 1; y <= m_height; ++y)
  {
    cdfV[y] = cdfV[y - 1] + funcV[y - 1];
  }
        
  const float integral = cdfV[m_height]; // The integral over this marginal CDF is in the last element.
  funcV[m_height] = integral;            // For completeness, actually unused.

  if (integral != 0.0f)
  {
    for (unsigned int y = 1; y <= m_height; ++y)
    {
      cdfV[y] /= integral;
    }
  }
  else // All texels were black in the whole image. Seriously? :-) Generate an equal distribution.
  {
    for (unsigned int y = 1; y <= m_height; ++y)
    {
      cdfV[y] = float(y) / float(m_height);
    }
  }

  // Upload that RGBA32F environment texture data.
  // Doing this here no not duplicate the code in the createEnvironment routines.
  m_buffer = context->createBuffer(RT_BUFFER_INPUT, m_format, m_width, m_height);
  m_buffer->setMipLevelCount(1);

  void *dst = m_buffer->map(0, RT_BUFFER_MAP_WRITE_DISCARD);
  memcpy(dst, m_texels.data(), m_width * m_height * sizeof(float) * 4);
  m_buffer->unmap();

  m_sampler = context->createTextureSampler();

  m_sampler->setWrapMode(0, RT_WRAP_REPEAT);
  m_sampler->setWrapMode(1, RT_WRAP_CLAMP_TO_EDGE); // Do not filter across the poles.
  m_sampler->setWrapMode(2, RT_WRAP_REPEAT);
  m_sampler->setFilteringModes(RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE);
  m_sampler->setIndexingMode(m_indexMode);
  m_sampler->setReadMode(m_readMode);
  m_sampler->setMaxAnisotropy(1.0f);
  m_sampler->setBuffer(0, 0, m_buffer);

  // Upload the CDFs into OptiX buffers.
  m_bufferCDF_U = context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT, m_width + 1, m_height); 

  void* buf = m_bufferCDF_U->map(0, RT_BUFFER_MAP_WRITE_DISCARD);
  memcpy(buf, cdfU, (m_width + 1) * m_height * sizeof(float));
  m_bufferCDF_U->unmap();

  m_bufferCDF_V = context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT, m_height + 1);

  buf = m_bufferCDF_V->map(0, RT_BUFFER_MAP_WRITE_DISCARD);
  memcpy(buf, cdfV, (m_height + 1) * sizeof(float));
  m_bufferCDF_V->unmap();

  delete [] cdfV;
  delete [] cdfU;
        
  delete [] funcV;
  delete [] funcU;

  m_texels.clear(); // The original float data is not needed anymore.

  return true;
}

float Texture::getIntegral() const
{
  // This is the sum of the piecewise linear function values (roughly the texels' intensity) divided by the number of texels m_width * m_height.
  return m_integral;
}

optix::Buffer Texture::getBufferCDF_U() const
{
  return m_bufferCDF_U;
}

optix::Buffer Texture::getBufferCDF_V() const
{
  return m_bufferCDF_V;
}
