// Copyright NVIDIA Corporation 2011
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// This code is part of the NVIDIA nvpro-pipeline https://github.com/nvpro-pipeline/pipeline

#include "inc/Timer.h"

#if defined(_WIN32)
#  define GETTIME(x) QueryPerformanceCounter(x)
#else     
#  define GETTIME(x) gettimeofday( x, 0 )
#endif 

Timer::Timer()
  : m_running(false)
  , m_seconds(0)
{
#if defined(_WIN32)
  QueryPerformanceFrequency(&m_freq);
#endif
}

Timer::~Timer()
{}

void Timer::start()
{
  if( !m_running )
  {
    m_running = true;
    // starting a timer: store starting time last
    GETTIME( &m_begin );
  }
}

void Timer::stop()
{
  // stopping a timer: store stopping time first
  Time tmp;
  GETTIME( &tmp );
  if( m_running )
  {
    m_seconds += calcDuration(m_begin, tmp);
    m_running = false;
  }
}

void Timer::reset()
{
  m_running = false;
  m_seconds = 0;
}

void Timer::restart()
{
  reset();
  start();
}

double Timer::getTime() const
{
  Time tmp;
  GETTIME( &tmp );
  if( m_running )
  {
    return m_seconds + calcDuration(m_begin, tmp);
  }
  else
  {
    return m_seconds;
  }
}

double Timer::calcDuration(Time begin, Time end) const
{
  double seconds;
#if defined(_WIN32)
  LARGE_INTEGER diff;
  diff.QuadPart = (end.QuadPart - begin.QuadPart);
  seconds = (double)diff.QuadPart / (double)m_freq.QuadPart;
#else
  timeval diff;
  if( begin.tv_usec <= end.tv_usec )
  {
    diff.tv_sec = end.tv_sec - begin.tv_sec;
    diff.tv_usec = end.tv_usec - begin.tv_usec;
  }
  else 
  {
    diff.tv_sec = end.tv_sec - begin.tv_sec - 1;
    diff.tv_usec = end.tv_usec - begin.tv_usec + (int)1e6;
  }
  seconds = diff.tv_sec + diff.tv_usec/1e6;
#endif
    return seconds;
}
