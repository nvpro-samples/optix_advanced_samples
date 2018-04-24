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


#include "inc/Picture.h"

#include <IL/il.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <iostream>

#include "inc/MyAssert.h"


static unsigned int numberOfComponents(int format)
{
  switch (format)
  {
    case IL_RGB:
    case IL_BGR:
      return 3;

    case IL_RGBA:
    case IL_BGRA:
      return 4;

    case IL_LUMINANCE: 
    case IL_ALPHA:
      return 1;

    case IL_LUMINANCE_ALPHA:
      return 2;

    default:
      MY_ASSERT(!"Unsupported image data format.");
      return 0;
  }
}

static unsigned int sizeOfComponents(int type)
{
  switch (type)
  {
    case IL_BYTE:
    case IL_UNSIGNED_BYTE:
      return 1;

    case IL_SHORT:
    case IL_UNSIGNED_SHORT:
      return 2;

    case IL_INT:
    case IL_UNSIGNED_INT:
    case IL_FLOAT:
      return 4;

    default:
      MY_ASSERT(!"Unsupported image data type.");
      return 0;
  }
}

Image::Image()
: m_width(0)
, m_height(0)
, m_depth(0)
, m_format(IL_RGBA)
, m_type(IL_UNSIGNED_BYTE)
, m_pixels(nullptr)
, m_bpp(0)
, m_bpl(0)
, m_bps(0)
, m_nob(0)
{
}


Image::Image(unsigned int width,
             unsigned int height,
             unsigned int depth,
             int          format,
             int          type)
: m_width(width)
, m_height(height)
, m_depth(depth)
, m_format(format)
, m_type(type)
, m_pixels(nullptr)
{
  m_bpp = numberOfComponents(m_format) * sizeOfComponents(m_type);
  m_bpl = m_width  * m_bpp;
  m_bps = m_height * m_bpl;
  m_nob = m_depth  * m_bps;
}

Image::~Image()
{
  if (m_pixels != nullptr)
  {
    delete[] m_pixels;
    m_pixels = nullptr;
  }
}

Image::Image(const Image& image)
: m_width(image.m_width)
, m_height(image.m_height)
, m_depth(image.m_depth)
, m_format(image.m_format)
, m_type(image.m_type)
, m_bpp(image.m_bpp)
, m_bpl(image.m_bpl)
, m_bps(image.m_bps)
, m_nob(image.m_nob)
, m_pixels(nullptr)
{
  if (image.m_pixels != nullptr) 
  {
    m_pixels = new unsigned char[image.m_nob];
    memcpy(m_pixels, image.m_pixels, image.m_nob); // Deep copy.
  }
}

static int determineFace(int i, bool isDDS, bool isCube)
{
  int face = i;
  // If this is a DDS cubemap exchange the z-negative and z-positive images to match OpenGL and what's used here for OptiX.
  if (isDDS && isCube)
  {
    if (i == 4)
    {
      face = 5;
    }
    else if (i == 5)
    {
      face = 4;
    }
  }
  return face;
}

static unsigned int numberOfMipmaps(unsigned int w, unsigned int h, unsigned int d)
{
  unsigned int bits = std::max(w, std::max(h, d));
  unsigned int i = 1;
  while (bits >>= 1) 
  {
    ++i;
  }
  return i;
}


Picture::Picture()
: m_isCube(false)
{
}

Picture::~Picture()
{
}

unsigned int Picture::getNumberOfImages() const
{
  return static_cast<unsigned int>(m_images.size());
}

unsigned int Picture::getNumberOfFaces(unsigned int indexImage) const
{
  MY_ASSERT(indexImage < m_images.size());
  return static_cast<unsigned int>(m_images[indexImage].size());
}

const Image* Picture::getImageFace(unsigned int indexImage, unsigned int indexFace) const
{
  if (indexImage < m_images.size() && indexFace < m_images[indexImage].size())
  {
    return &m_images[indexImage][indexFace];
  }
  return nullptr;
}

bool Picture::isCubemap() const
{
  return m_isCube;
}

bool Picture::load(const std::string& filename)
{
  bool success = false;

  m_images.clear(); // Each load() wipes previously loaded image data.

  std::string foundFile = filename; // DAR FIXME Search at least the current working directory.
  if (foundFile.empty())
  {
    std::cerr << "ERROR Image::load(): " << filename << " not found" << std::endl;
    MY_ASSERT(!"Image not found");
    return success;
  }

  std::string ext;
  std::string::size_type last = filename.find_last_of('.');
  if (last != std::string::npos) 
  { 
    ext = filename.substr(last, std::string::npos);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return std::tolower(c); });
  }

  bool isDDS = (ext == std::string(".dds")); // .dds images need special handling
  m_isCube = false;
  
  unsigned int imageID;

  ilGenImages(1, (ILuint *) &imageID);
  ilBindImage(imageID);

  // Let DevIL handle the proper orientation during loading.
  if (isDDS)
  {
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_UPPER_LEFT); // DAR DEBUG What happens when I set IL_ORIGIN_LOWER_LEFT all the time?
  }
  else
  {
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
  }

  // load the image from file
  if (ilLoadImage((const ILstring) foundFile.c_str()))
  {
    // Querying for IL_NUM_IMAGES returns the number of images following the current one. Add 1 for the right image count!
    int numImages = ilGetInteger(IL_NUM_IMAGES) + 1;

    int numMipmaps = ilGetInteger(IL_NUM_MIPMAPS);

    std::vector<const void*> mipmaps; // Empty after instantiation.

    // Special check to see if the number of images build a mipmap chain
    // check if we have mipmaps as top-level images
    // NOTE: this, for example, could apply for tiff images
    if (1 < numImages && !numMipmaps)
    {
      bool failed = false; // indicates if test below has failed
      unsigned int w = 0;  // width of the predecessor image
      
      for (int i = 0; i < numImages; ++i)
      {
        ilBindImage(imageID);  
        ilActiveImage(i);
        MY_ASSERT(IL_NO_ERROR == ilGetError()); 

        unsigned int ww = ilGetInteger(IL_IMAGE_WIDTH); // actual image width
        if (0 < i) // start checking with the second image
        {
          if (ww == (w >> 1)) // criteria for next mipmap level
          { 
            // top-level image actually is the i-th mipmap level
            mipmaps.push_back(ilGetData()); 
          } 
          else
          {
            // could not identify top-level image as a mipmap level
            // --> no further testing required
            failed = true; // test failed, means the number of images do not build a mipmap chain.
            break; 
          }
        }
        w = ww; // prepare next-level test
      }
      if (!failed && !mipmaps.empty())
      {
        // reset numImages to 1 in case we identified all but 
        // the first top-level image as actually being mipmap levels
        numImages = 1;
        // DAR FIXME Should this call mipmaps.clear() here.
      }
    }

    m_isCube = (ilGetInteger(IL_IMAGE_CUBEFLAGS) != 0) || (numImages == 6);

    for (int image = 0; image < numImages; ++image)
    {
      // cube faces within DevIL philosophy are organized like this:
      //
      //   image -> 1st face -> face index 0
      //   face1 -> 2nd face -> face index 1
      //   ...
      //   face5 -> 6th face -> face index 5

      int numFaces = ilGetInteger(IL_NUM_FACES) + 1;

      for (int f = 0; f < numFaces; ++f)
      {
        // Need to juggle with the faces to get them aligned with
        // how OpenGL expects cube faces. (Using the same layout in OptiX.)
        int face = determineFace(f, isDDS, m_isCube);

        // DevIL frequently loses track of the current state.
        ilBindImage(imageID);
        ilActiveImage(image);
        ilActiveFace(face);

        MY_ASSERT(IL_NO_ERROR == ilGetError()); 

        // pixel format
        int format = ilGetInteger(IL_IMAGE_FORMAT);

        if (IL_COLOR_INDEX == format)
        {
          // convert color index to whatever the base type of the palette is
          if (!ilConvertImage(ilGetInteger(IL_PALETTE_BASE_TYPE), IL_UNSIGNED_BYTE))
          {
            // Free all resources associated with the DevIL image.
            ilDeleteImages(1, &imageID);
            MY_ASSERT(IL_NO_ERROR == ilGetError());
            return false;
          }
          // now query format of the converted image
          format = ilGetInteger(IL_IMAGE_FORMAT);
        }

        int type = ilGetInteger(IL_IMAGE_TYPE);

        // image dimension in pixels
        unsigned int width  = ilGetInteger(IL_IMAGE_WIDTH);
        unsigned int height = ilGetInteger(IL_IMAGE_HEIGHT);
        unsigned int depth  = ilGetInteger(IL_IMAGE_DEPTH);

        if (width == 0) // There must be at least a single pixel.
        {
          MY_ASSERT(!"Image::load() Image with zero dimension");
          // Free all resources associated with the DevIL image.
          ilDeleteImages(1, &imageID);
          MY_ASSERT(IL_NO_ERROR == ilGetError());
          return false;
        }

        // Adjust the input data to at least a single texel.
        width  += (width  == 0);
        height += (height == 0);
        depth  += (depth  == 0);

        unsigned int index = addImage(width, height, depth, format, type);

        // Use mipmaps, if any, in the original file
        if (0 < numMipmaps)
        {
          mipmaps.clear(); // clear this for currently processed image

          for (int j = 1; j <= numMipmaps; ++j)
          { 
            // DevIL frequently loses track of the current state
            ilBindImage(imageID);
            ilActiveImage(image);
            ilActiveFace(face);
            ilActiveMipmap(j);

            mipmaps.push_back((const void*) ilGetData());
          }

          // DevIL frequently loses track of the current state
          ilBindImage(imageID);
          ilActiveImage(image);
          ilActiveFace(face);
          ilActiveMipmap(0);
        }

        setImageData(index, (const void*) ilGetData(), mipmaps);

        if (isDDS && m_isCube)
        {
          // WARNING:
          // This piece of code MUST NOT be visited twice for the same image,
          // because this would falsify the desired effect!
          // The images at this position are flipped at the x-axis (due to DevIL)
          // flipping at x-axis will result in original image
          // mirroring at y-axis will result in rotating the image 180 degree
          if (face == 0 || face == 1 || face == 4 || face == 5) // px, nx, pz, nz
          {
            mirrorY(index); // mirror over y-axis
          }
          else // py, ny
          {
            mirrorX(index); // flip over x-axis
          }
        }

        ILint origin = ilGetInteger(IL_IMAGE_ORIGIN);
        if (!m_isCube && origin == IL_ORIGIN_UPPER_LEFT)
        {
          // OpenGL expects origin at lower left, so the image has to be flipped at the x-axis
          // for DDS cubemaps we handle the separate face rotations above
          // DAR FIXME This should only happen for DDS images. 
          // All others are flipped by DevIL because I set the origin to lower left. Handle DDS images the same?
          mirrorX(index); // reverse rows 
        }
      }
    }
    success = true;
  }

  // free all resources associated with the DevIL image
  ilDeleteImages(1, &imageID);
  MY_ASSERT(IL_NO_ERROR == ilGetError());
  
  return success;
}

void Picture::clear()
{
  m_images.clear();
}


// Private functions 

unsigned int Picture::addImage(unsigned int width,
                               unsigned int height,
                               unsigned int depth,
                               int          format,
                               int          type)
{
  MY_ASSERT((0 < width) && (0 < height) && (0 < depth));

  m_images.push_back(std::vector<Image>());
  m_images.back().push_back(Image(width, height, depth, format, type));
 
  return (unsigned int)(m_images.size() - 1);
}

bool Picture::copyMipmaps(unsigned int index, std::vector<const void*> const& mipmaps)
{
  std::vector<Image>& images = m_images[index];
  
  MY_ASSERT(images.size()); // must hold at least the top level image
  
  // Release existing mipmaps.
  if (1 < images.size())
  {
    for (size_t i = 1; i < images.size(); ++i)
    {
      delete[] images[i].m_pixels;
    }
    images.resize(1);
  }

  unsigned int numMipmaps = numberOfMipmaps(images[0].m_width, images[0].m_height, images[0].m_depth); // Includes LOD 0.
  images.reserve(numMipmaps); // This needs the deep copy constructor!
  
  // This demo doesn't contain code to generate mipmaps from arbitrary input data formats.
  // If the provided number of mipmaps doesn't match the required number, keep only the LOD 0 image.
  if (numMipmaps != mipmaps.size() + 1) // numMipmaps contains the base image LOD 0 as well, mipmaps doesn't.
  {
    MY_ASSERT(!"Number of required mipmaps does not match number of provided mipmaps.");
    return false; // This will leave the mipmaps unpopulated. There is no mipmap generation code present in this demo.
  }

  Image* src = &images[0];

  unsigned int w = src->m_width;
  unsigned int h = src->m_height;
  unsigned int d = src->m_depth;
  
  int format = src->m_format;
  int type   = src->m_type;

  for (size_t i = 0; i < mipmaps.size() && (1 < w || 1 < h || 1 < d); ++i)
  {
    MY_ASSERT(mipmaps[i]);

    // Calculate the next mipmap level size.
    w = (1 < w) ? w >> 1 : 1;
    h = (1 < h) ? h >> 1 : 1;
    d = (1 < d) ? d >> 1 : 1;
    
    // append the next level image to the images
    images.push_back(Image(w, h, d, format, type));

    Image* dst = &images.back(); // points to newly created Image

    dst->m_pixels = new unsigned char[dst->m_nob];
    memcpy(dst->m_pixels, mipmaps[i], dst->m_nob);
  }
  return true; // succeeded if we get here
}

void Picture::setImageData(unsigned int index, const void* pixels, std::vector<const void*> const& mipmaps)
{
  MY_ASSERT(index < m_images.size());

  Image* image = &m_images[index][0]; // LOD 0 image

  if (image->m_pixels)
  {
    delete[] image->m_pixels;
    image->m_pixels = nullptr;
  }

  image->m_pixels = new unsigned char[image->m_nob];

  memcpy(image->m_pixels, pixels, image->m_nob);

  // Consider mipmaps passed through mipmaps vector.
  if (!mipmaps.empty())
  {
    copyMipmaps(index, mipmaps);
  }
}

void Picture::mirrorX(unsigned int index)
{
  MY_ASSERT(index < m_images.size());

  // Flip all images upside down.
  for (size_t i = 0; i < m_images[index].size(); ++i)
  {
    Image* image = &m_images[index][i];

    const unsigned char* srcPixels = image->m_pixels;
    unsigned char*       dstPixels = new unsigned char[image->m_nob];

    for (unsigned int z = 0; z < image->m_depth; ++z) 
    {
      for (unsigned int y = 0; y < image->m_height; ++y) 
      {
        const unsigned char* srcLine = srcPixels + z * image->m_bps + y * image->m_bpl;
        unsigned char*       dstLine = dstPixels + z * image->m_bps + (image->m_height - 1 - y) * image->m_bpl;
      
        memcpy(dstLine, srcLine, image->m_bpl);
      }
    }
    delete[] image->m_pixels;
    image->m_pixels = dstPixels;
  }
}

void Picture::mirrorY(unsigned int index)
{
  MY_ASSERT(index < m_images.size());

  // Mirror all images left to right.
  for (size_t i = 0; i < m_images[index].size(); ++i)
  {
    Image* image = &m_images[index][i];

    const unsigned char* srcPixels = image->m_pixels;
    unsigned char*       dstPixels = new unsigned char[image->m_nob];

    for (unsigned int z = 0; z < image->m_depth; ++z) 
    {
      for (unsigned int y = 0; y < image->m_height; ++y) 
      {
        const unsigned char* srcLine = srcPixels + z * image->m_bps + y * image->m_bpl;
        unsigned char*       dstLine = dstPixels + z * image->m_bps + y * image->m_bpl;

        for (unsigned int x = 0; x < image->m_width; ++x) 
        {
          const unsigned char* srcPixel = srcLine + x * image->m_bpp;
          unsigned char*       dstPixel = dstLine + (image->m_width - 1 - x) * image->m_bpp;

          memcpy(dstPixel, srcPixel, image->m_bpp);
        }
      }
    }

    delete[] image->m_pixels;
    image->m_pixels = dstPixels;
  }
}

