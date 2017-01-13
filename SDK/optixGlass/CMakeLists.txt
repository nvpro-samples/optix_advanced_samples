
#
#  Copyright (c) 2008 - 2009 NVIDIA Corporation.  All rights reserved.
#
#  NVIDIA Corporation and its licensors retain all intellectual property and proprietary
#  rights in and to this software, related documentation and any modifications thereto.
#  Any use, reproduction, disclosure or distribution of this software and related
#  documentation without an express license agreement from NVIDIA Corporation is strictly
#  prohibited.
#
#  TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS*
#  AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED,
#  INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
#  PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY
#  SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT
#  LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
#  BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR
#  INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF
#  SUCH DAMAGES
#

# For commonStructs.h
include_directories(${SAMPLES_CUDA_DIR})

# See top level CMakeLists.txt file for documentation of OPTIX_add_sample_executable.
OPTIX_add_sample_executable( optixGlass
    optixGlass.cpp

    ${SAMPLES_CUDA_DIR}/accum_camera.cu
    ${SAMPLES_CUDA_DIR}/diffuse_grid.cu
    ${SAMPLES_CUDA_DIR}/glass.cu
    ${SAMPLES_CUDA_DIR}/gradientbg.cu
    ${SAMPLES_CUDA_DIR}/parallelogram_iterative.cu
    ${SAMPLES_CUDA_DIR}/triangle_mesh_iterative.cu
    ${SAMPLES_CUDA_DIR}/commonStructs.h
    ${SAMPLES_CUDA_DIR}/helpers.h
    ${SAMPLES_CUDA_DIR}/prd.h
    )

