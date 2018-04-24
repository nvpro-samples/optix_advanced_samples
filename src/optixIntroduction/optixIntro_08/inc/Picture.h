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

// Code in these classes is based on the ILTexLoader.h/.cpp routines inside the NVIDIA nvpro-pipeline ILTexLoader plugin:
// https://github.com/nvpro-pipeline/pipeline/blob/master/dp/sg/io/IL/Loader/ILTexLoader.cpp

#pragma once

#ifndef PICTURE_H
#define PICTURE_H

#include <string>
#include <vector>

struct Image
{
  Image();
  Image(const Image& image);
  Image(unsigned int width, unsigned int height, unsigned int depth, int format, int type);
  ~Image();
  
  unsigned int m_width;
  unsigned int m_height;
  unsigned int m_depth;

  int          m_format; // DevIL image format.
  int          m_type;   // DevIL image component type.

  // Derived values.
  unsigned int m_bpp; // bytes per pixel
  unsigned int m_bpl; // bytes per scanline
  unsigned int m_bps; // bytes per slice (plane)
  unsigned int m_nob; // number of bytes (complete image)

  unsigned char* m_pixels; // The pixel data of one image.
};


class Picture
{
public:
  Picture();
  ~Picture();

  bool load(const std::string& filename);
  void clear();

  unsigned int getNumberOfImages() const;
  unsigned int getNumberOfFaces(unsigned int indexImage) const;
  const Image* getImageFace(unsigned int indexImage, unsigned int indexFace) const;
  bool isCubemap() const;

private:
  unsigned int addImage(unsigned int width, unsigned int height, unsigned int depth, int format, int type);
  bool copyMipmaps(unsigned int index, std::vector<const void*> const& mipmaps);
  void setImageData(unsigned int index, const void* pixels, std::vector<const void*> const& mipmaps);
  void mirrorX(unsigned int index);
  void mirrorY(unsigned int index);

private:
  bool m_isCube; // Track if the picture is a cube map.
  std::vector< std::vector<Image> > m_images;
};

#endif // PICTURE_H
