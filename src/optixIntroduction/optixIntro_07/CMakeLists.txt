# 
# Copyright (c) 2013-2018, NVIDIA CORPORATION. All rights reserved.
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

# See top level CMakeLists.txt file for documentation of OPTIX_add_sample_executable.

OPTIX_add_sample_executable( optixIntro_07
  src/main.cpp

  inc/Application.h
  src/Application.cpp

  src/Box.cpp
  src/Parallelogram.cpp
  src/Plane.cpp
  src/Sphere.cpp
  src/Torus.cpp

  inc/LensShader.h

  inc/PinholeCamera.h
  src/PinholeCamera.cpp

  inc/Picture.h
  src/Picture.cpp

  inc/Texture.h
  src/Texture.cpp

  inc/Timer.h
  src/Timer.cpp

  inc/MyAssert.h

  shaders/app_config.h
  shaders/function_indices.h
  shaders/per_ray_data.h
  shaders/material_parameter.h
  shaders/random_number_generators.h
  shaders/light_definition.h
  shaders/rt_assert.h
  shaders/rt_function.h
  shaders/shader_common.h
  shaders/vertex_attributes.h

  shaders/boundingbox_triangle_indexed.cu
  shaders/intersection_triangle_indexed.cu

  shaders/closesthit.cu
  shaders/closesthit_light.cu
  shaders/anyhit.cu

  shaders/raygeneration.cu
  shaders/exception.cu
  shaders/miss.cu

  shaders/lens_shader.cu
  shaders/bsdf_diffuse_reflection.cu
  shaders/bsdf_specular_reflection.cu
  shaders/bsdf_specular_reflection_transmission.cu
  shaders/light_sample.cu
)    

include_directories(
  "."
  ${IL_INCLUDE_DIR}
)

target_link_libraries( optixIntro_07
  ${IL_LIBRARIES}
  ${ILU_LIBRARIES}
  ${ILUT_LIBRARIES}
)

