// Copyright NVIDIA Corporation 2002-2005
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

#pragma once
 
#ifndef TIMER_H
#define TIMER_H

#if defined(_WIN32)
  #include <Windows.h>
#else
  #include <sys/time.h>
#endif


/*! \brief A simple timer class.
  * This timer class can be used on Windows and Linux systems to
  * measure time intervals in seconds. 
  * The timer can be started and stopped several times and accumulates
  * time elapsed between the start() and stop() calls. */
class Timer
{
public:
  //! Default constructor. Constructs a Timer, but does not start it yet. 
  Timer();

  //! Default destructor.
  ~Timer();

  //! Starts the timer.
  void start();

  //! Stops the timer.
  void stop();

  //! Resets the timer.
  void reset();

  //! Resets the timer and starts it.
  void restart();

  //! Returns the current time in seconds.
  double getTime() const;

  //! Return whether the timer is still running.
  bool isRunning() const { return m_running; }

private:
#if defined(_WIN32)
  typedef LARGE_INTEGER Time;
#else
  typedef timeval Time;
#endif

private:
  double calcDuration(Time begin, Time end) const;

private:
#if defined(_WIN32)
  LARGE_INTEGER m_freq;
#endif
  Time   m_begin;
  bool   m_running;
  double m_seconds;
};

#endif // TIMER_H
