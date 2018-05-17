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

#include "inc/PinholeCamera.h"

#include <iostream>


PinholeCamera::PinholeCamera()
: m_distance(10.0f) // Some camera defaults for the demo scene.
, m_phi(0.75f)
, m_theta(0.6f)
, m_fov(60.0f)
//:  m_distance(3.0f)
//, m_phi(0.75f)  // positive z-axis
//, m_theta(0.5f) // equator
//, m_fov(60.0f)
, m_width(1)
, m_height(1)
, m_aspect(1.0f)
, m_baseX(0)
, m_baseY(0)
, m_speedRatio(10.0f)
, m_dx(0)
, m_dy(0)
, m_changed(false)
{
  m_center = optix::make_float3(0.0f, 0.0f, 0.0f);

  m_cameraPosition = optix::make_float3(0.0f, 0.0f, 1.0f);
  m_cameraU        = optix::make_float3(1.0f, 0.0f,  0.0f);
  m_cameraV        = optix::make_float3(0.0f, 1.0f,  0.0f);
  m_cameraW        = optix::make_float3(0.0f, 0.0f, -1.0f);
}

PinholeCamera::~PinholeCamera()
{
}

void PinholeCamera::setViewport(int w, int h)
{
  if (m_width != w || m_height != h) 
  {
    // Never drop to zero viewport size. This avoids lots of checks for zero in other routines.
    m_width  = (w) ? w : 1;
    m_height = (h) ? h : 1;
    m_aspect = float(m_width) / float(m_height);
    m_changed = true;
  }
}

void PinholeCamera::setBaseCoordinates(int x, int y)
{
  m_baseX = x;
  m_baseY = y;
}

void PinholeCamera::orbit(int x, int y)
{
  if (setDelta(x, y))
  {
    m_phi -= float(m_dx) / float(m_width); // Inverted.
    // Wrap phi.
    if (m_phi < 0.0f)
    {
      m_phi += 1.0f; 
    }
    else if (1.0f < m_phi)
    {
      m_phi -= 1.0f; 
    }

    m_theta += float(m_dy) / float(m_height);
    // Clamp theta.
    if (m_theta < 0.0f)
    {
      m_theta = 0.0f; 
    }
    else if (1.0f < m_theta)
    {
      m_theta = 1.0f; 
    }
  }
}

void PinholeCamera::pan(int x, int y)
{
  if (setDelta(x, y))
  {
    // m_speedRatio pixels will move one vector length.
    float u = float(m_dx) / m_speedRatio;
    float v = float(m_dy) / m_speedRatio;
    // Pan the center of interest, the rest will follow.
    m_center = m_center - u * m_cameraU + v * m_cameraV;
  }
}

void PinholeCamera::dolly(int x, int y)
{
  if (setDelta(x, y))
  {
    // m_speedRatio pixels will move one vector length.
    float w = float(m_dy) / m_speedRatio;
    // Adjust the distance, the center of interest stays fixed so that the orbit is around the same center.
    m_distance -= w * length(m_cameraW); // Dragging down moves the camera forwards. "Drag-in the object".
    if (m_distance < 0.001f) // Avoid swapping sides. Scene units are meters [m].
    {
      m_distance = 0.001f;
    }
  }
}

void PinholeCamera::focus(int x, int y)
{
  if (setDelta(x, y))
  {
    // m_speedRatio pixels will move one vector length.
    float w = float(m_dy) / m_speedRatio;
    // Adjust the center of interest.
    setFocusDistance(m_distance - w * length(m_cameraW));
  }
}

void PinholeCamera::setFocusDistance(float f)
{
  if (m_distance != f && 0.001f < f) // Avoid swapping sides.
  {
    m_distance = f;
    m_center = m_cameraPosition + m_distance * m_cameraW; // Keep the camera position fixed and calculate a new center of interest which is the focus plane.
    m_changed = true; // m_changed is only reset when asking for the frustum
  }
}

void PinholeCamera::zoom(float x)
{
  m_fov += float(x);
  if (m_fov < 1.0f)
  {
    m_fov = 1.0f;
  }
  else if (179.0 <  m_fov)
  {
    m_fov = 179.0f;
  }
  m_changed = true;
}

float PinholeCamera::getAspectRatio() const
{
  return m_aspect;
}

bool PinholeCamera::getFrustum(optix::float3& pos, optix::float3& u, optix::float3& v, optix::float3& w)
{
  bool changed = m_changed;
  if (changed)
  {
    // Recalculate the camera parameters.
    const float cosPhi   = cosf(m_phi * 2.0f * M_PIf);
    const float sinPhi   = sinf(m_phi * 2.0f * M_PIf);
    const float cosTheta = cosf(m_theta * M_PIf);
    const float sinTheta = sinf(m_theta * M_PIf);
  
    optix::float3 normal = optix::make_float3(cosPhi * sinTheta, -cosTheta, -sinPhi * sinTheta); // "normal", unit vector from origin to spherical coordinates (phi, theta)

    float tanFov = tanf((m_fov * 0.5f) * M_PIf / 180.0f); // m_fov is in the range [1.0f, 179.0f].
    m_cameraPosition = m_center + m_distance * normal;

    m_cameraU = m_aspect * optix::make_float3(-sinPhi, 0.0f, -cosPhi) * tanFov;               // "tangent"
    m_cameraV = optix::make_float3(cosTheta * cosPhi, sinTheta, cosTheta * -sinPhi) * tanFov; // "bitangent"
    m_cameraW = -normal;                                                                   // "-normal" to look at the center.

    pos = m_cameraPosition;
    u = m_cameraU;
    v = m_cameraV;
    w = m_cameraW;

    m_changed = false; // Next time asking for the frustum will return false unless the camera has changed again.
  }
  return changed;
}

bool PinholeCamera::setDelta(int x, int y)
{
  if (m_baseX != x || m_baseY != y) 
  {
    m_dx = x - m_baseX;
    m_dy = y - m_baseY;

    m_baseX = x; 
    m_baseY = y;

    m_changed = true; // m_changed is only reset when asking for the frustum.
    return true; // There is a delta.
  }
  return false;
}

void PinholeCamera::setSpeedRatio(float f)
{
  m_speedRatio = f;
  if (m_speedRatio < 0.01f)
  {
    m_speedRatio = 0.01f;
  }
  else if (1000.0f < m_speedRatio)
  {
    m_speedRatio = 1000.0f;
  }
}
