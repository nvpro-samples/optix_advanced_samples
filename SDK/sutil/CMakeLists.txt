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

set(sources
  ${SAMPLES_CUDA_DIR}/commonStructs.h
  ${SAMPLES_CUDA_DIR}/helpers.h
  ${SAMPLES_CUDA_DIR}/intersection_refinement.h
  ${SAMPLES_CUDA_DIR}/phong.h
  ${SAMPLES_CUDA_DIR}/phong.cu
  ${SAMPLES_CUDA_DIR}/random.h
  ${SAMPLES_CUDA_DIR}/triangle_mesh.cu
  rply-1.01/rply.c
  rply-1.01/rply.h
  Arcball.cpp
  Arcball.h
  HDRLoader.cpp
  HDRLoader.h
  Mesh.cpp
  Mesh.h
  OptiXMesh.cpp
  OptiXMesh.h
  PPMLoader.cpp
  PPMLoader.h
  ${CMAKE_CURRENT_BINARY_DIR}/../sampleConfig.h
  sutil.cpp
  sutil.h
  sutilapi.h
  tinyobjloader/tiny_obj_loader.cc
  tinyobjloader/tiny_obj_loader.h
  )

if(OPENGL_FOUND AND NOT APPLE)
  list(APPEND sources "glew.c" "GL/glew.h")
  if( WIN32 )
    list(APPEND sources "GL/wglew.h")
  else()
    list(APPEND sources "GL/glxew.h")
  endif()

  if( WIN32 )
    add_definitions(-DGLEW_BUILD)
  endif()

  if(USING_GNU_CXX OR USING_CLANG_CXX)
    # glew.c #includes glew.h which contains "lax" function prototypes
    set_source_files_properties("glew.c" PROPERTIES COMPILE_FLAGS  "-Wno-strict-prototypes -Wno-missing-prototypes")

  endif()
  
endif()

if(GLUT_FOUND AND OPENGL_FOUND)
  include_directories(${GLUT_INCLUDE_DIR})
  add_definitions(-DGLUT_FOUND -DGLUT_NO_LIB_PRAGMA)

endif()


include_directories(${CMAKE_CURRENT_SOURCE_DIR})
# For commonStructs.h
include_directories(${SAMPLES_CUDA_DIR})

# Compile the cuda files to ptx.  Note that this will ignore all of the non CUDA
# files.
CUDA_COMPILE_PTX(ptx_files ${sources})

# Make the library.
set(sutil_target "sutil_sdk")
add_library(${sutil_target} ${sources} ${ptx_files})

# Use gcc rather than g++ to link if we are linking statically against libgcc_s and libstdc++
if(USING_GNU_C OR USING_GNU_CXX)
  if(GCC_LIBSTDCPP_HACK)
    set_target_properties(${sutil_target} PROPERTIES LINKER_LANGUAGE "C")
    target_link_libraries(${sutil_target} LINK_PRIVATE ${STATIC_LIBSTDCPP})
  endif()
endif()

# Note that if the GLUT_LIBRARIES and OPENGL_LIBRARIES haven't been looked for,
# these variable will be empty.
target_link_libraries(${sutil_target}
  optix
  optixu
  ${GLUT_LIBRARIES}
  ${OPENGL_LIBRARIES}
  )
if(WIN32)
  target_link_libraries(${sutil_target} winmm.lib)
endif()
add_perforce_to_target( ${sutil_target} )

# Copy the free glut dlls as part of the sutil build process
if(WIN32)
  if(CMAKE_GENERATOR MATCHES "Visual Studio")
    set( build_configuration "$(ConfigurationName)" )
  else()
    set( build_configuration "${CMAKE_BUILD_TYPE}")
  endif()

  add_custom_command(TARGET ${sutil_target}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    -D build_configuration:STRING=${build_configuration}
    -D "output_directory:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}"
    -D "source_dll:FILE=${GLUT_glut_DLL}"
    -P "${CMAKE_SOURCE_DIR}/CMake/CopyDLL.cmake"
    )
endif()

if(RELEASE_INSTALL_BINARY_SAMPLES AND NOT RELEASE_STATIC_BUILD)
  # If performing a release install, we want to use rpath for our install name.
  # The executables' rpaths will then be set to @executable_path so we can invoke
  # the samples from an arbitrary location and it will still find this library.
  set_target_properties(${sutil_target} PROPERTIES
    INSTALL_NAME_DIR "@rpath"
    BUILD_WITH_INSTALL_RPATH ON
    )
  install(TARGETS ${sutil_target}
    RUNTIME DESTINATION ${SDK_BINARY_INSTALL_DIR}
    LIBRARY DESTINATION ${SDK_BINARY_INSTALL_DIR}
    )
endif()

# Make the list of sources available to the parent directory for installation needs.
set(sutil_sources "${sources}" PARENT_SCOPE)
