/* 
 * Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.
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


#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>
#include <optixu/optixu_aabb_namespace.h>

#include "intersection_refinement.h"

using namespace optix;

rtBuffer< optix::Aabb > box_buffer;
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );

rtDeclareVariable( float3, back_hit_point, attribute back_hit_point, );
rtDeclareVariable(float3, front_hit_point, attribute front_hit_point, );
rtDeclareVariable(float3, texcoord, attribute texcoord, ); 
rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, ); 
rtDeclareVariable(float3, shading_normal, attribute shading_normal, ); 

static __device__ float3 boxnormal(float3 boxmin, float3 boxmax, float t)
{
    float3 t0 = (boxmin - ray.origin)/ray.direction;
    float3 t1 = (boxmax - ray.origin)/ray.direction;
    float3 neg = make_float3(t==t0.x?1:0, t==t0.y?1:0, t==t0.z?1:0);
    float3 pos = make_float3(t==t1.x?1:0, t==t1.y?1:0, t==t1.z?1:0);
    return pos-neg;
}

RT_PROGRAM void intersect( int primId )
{
    const float3 boxmin = box_buffer[primId].m_min;
    const float3 boxmax = box_buffer[primId].m_max;
    float3 t0 = (boxmin - ray.origin)/ray.direction;
    float3 t1 = (boxmax - ray.origin)/ray.direction;
    float3 near = fminf(t0, t1);
    float3 far = fmaxf(t0, t1);
    float tmin = fmaxf( near );
    float tmax = fminf( far );

    if(tmin <= tmax) {
        bool check_second = true;
        if( rtPotentialIntersection( tmin ) ) {
            texcoord = make_float3( 0.0f );
            shading_normal = geometric_normal = boxnormal( boxmin, boxmax, tmin );

            // TODO: refine
            const float3 p = ray.origin + tmin*ray.direction;
            back_hit_point = front_hit_point = p;

            if(rtReportIntersection(0))
                check_second = false;
        } 
        if(check_second) {
            if( rtPotentialIntersection( tmax ) ) {
                texcoord = make_float3( 0.0f );
                shading_normal = geometric_normal = boxnormal( boxmin, boxmax, tmax );

                // TODO: refine
                const float3 p = ray.origin + tmax*ray.direction;
                back_hit_point = front_hit_point = p;

                rtReportIntersection(0);
            }
        }
    }
}

RT_PROGRAM void bounds (int primId, float result[6])
{
    optix::Aabb* aabb = (optix::Aabb*)result;
    optix::Aabb box = box_buffer[primId]; 
    aabb->set( box.m_min, box.m_max );
}

