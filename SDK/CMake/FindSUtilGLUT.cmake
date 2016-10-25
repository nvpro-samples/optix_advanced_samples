#
# Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

# Finds sutil's copies of the libraries before looking around for the system libraries on Windows platforms.

IF (WIN32)
  SET(GLUT_ROOT_PATH $ENV{GLUT_ROOT_PATH})
ENDIF (WIN32)

find_package(OpenGL)

if(WIN32)
  # For whatever reason, cmake doesn't detect that a library is 32 or 64 bits,
  # so we have to selectively look for it in one of two places.
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(dir win64)
  else() # 32 bit
    set(dir win32)
  endif()
  find_library(GLUT_glut_LIBRARY names freeglut
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/support/freeglut/${dir}/Release
    NO_DEFAULT_PATH
    )
  find_file(GLUT_glut_DLL names freeglut.dll
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/support/freeglut/${dir}/Release
    NO_DEFAULT_PATH
    )
  find_path(GLUT_INCLUDE_DIR GL/glut.h
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/support/freeglut/include
    )
  if( GLUT_glut_LIBRARY AND
      GLUT_glut_DLL     AND
      GLUT_INCLUDE_DIR
      )
    # We need to set some of the same variables that FindGLUT.cmake does.
    set(GLUT_FOUND TRUE)
    set(GLUT_LIBRARIES "${GLUT_glut_LIBRARY}"
      #winmm.lib
      )
    set(sources ${sources} ${GLUT_INCLUDE_DIR}/GL/glut.h)

  endif() # All the components were found

  # Mark the libraries as advanced
  mark_as_advanced(
    GLUT_glut_LIBRARY
    GLUT_glut_DLL
    GLUT_INCLUDE_DIR
    )
else() # Now for everyone else
  find_package(GLUT REQUIRED)
  # Some systems don't need Xmu for glut.  Remove it if it wasn't found.
  if( DEFINED GLUT_Xmu_LIBRARY AND NOT GLUT_Xmu_LIBRARY)
    list(REMOVE_ITEM GLUT_LIBRARIES ${GLUT_Xmu_LIBRARY})
  endif()
endif()
