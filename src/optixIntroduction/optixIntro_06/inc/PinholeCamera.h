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

#ifndef PINHOLE_CAMERA_H
#define PINHOLE_CAMERA_H

#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>


class PinholeCamera
{
public:
  PinholeCamera();
  ~PinholeCamera();

  void setViewport(int w, int h);
  void setBaseCoordinates(int x, int y);
  void setSpeedRatio(float f);
  void setFocusDistance(float f);

  void orbit(int x, int y);
  void pan(int x, int y);
  void dolly(int x, int y);
  void focus(int x, int y);
  void zoom(float x);

  bool  getFrustum(optix::float3& pos, optix::float3& u, optix::float3& v, optix::float3& w);
  float getAspectRatio() const;
  
public: // Just to be able to load and save them easily.
  optix::float3 m_center;   // Center of interest point, around which is orbited (and the sharp plane of a depth of field camera).
  float         m_distance; // Distance of the camera from the center of intest.
  float         m_phi;      // Range [0.0f, 1.0f] from positive x-axis 360 degrees around the latitudes.
  float         m_theta;    // Range [0.0f, 1.0f] from negative to positive y-axis.
  float         m_fov;      // In degrees. Default is 60.0f

private:
  bool setDelta(int x, int y);
  
private:
  int   m_width;    // Viewport width.
  int   m_height;   // Viewport height.
  float m_aspect;   // m_width / m_height
  int   m_baseX;
  int   m_baseY;
  float m_speedRatio;
  
  // Derived values:
  int           m_dx;
  int           m_dy;
  bool          m_changed;
  optix::float3 m_cameraPosition;
  optix::float3 m_cameraU;
  optix::float3 m_cameraV;
  optix::float3 m_cameraW;
};

#endif // PINHOLE_CAMERA_H
