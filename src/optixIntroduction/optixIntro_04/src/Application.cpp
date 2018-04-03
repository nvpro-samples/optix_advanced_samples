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

#include "shaders/app_config.h"

#include "inc/Application.h"

#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>

#include "shaders/material_parameter.h"

// DAR Only for sutil::samplesPTXDir() and sutil::writeBufferToFile()
#include <sutil.h>

#include "inc/MyAssert.h"

const char* const SAMPLE_NAME = "optixIntro_04";

// This only runs inside the OptiX Advanced Samples location,
// unless the environment variable OPTIX_SAMPLES_SDK_PTX_DIR is set.
// A standalone application which should run anywhere would place the *.ptx files 
// into a subdirectory next to the executable and use a relative file path here!
static std::string ptxPath(std::string const& cuda_file)
{
  return std::string(sutil::samplesPTXDir()) + std::string("/") + 
         std::string(SAMPLE_NAME) + std::string("_generated_") + cuda_file + std::string(".ptx");
}


Application::Application(GLFWwindow* window,
                         const int width,
                         const int height,
                         const unsigned int devices, 
                         const unsigned int stackSize,
                         const bool interop)
: m_window(window)
, m_width(width)
, m_height(height)
, m_devicesEncoding(devices)
, m_stackSize(stackSize)
, m_interop(interop)
{
  // Setup ImGui binding.
  ImGui::CreateContext();
  ImGui_ImplGlfwGL2_Init(window, true);

  // This initializes the GLFW part including the font texture.
  ImGui_ImplGlfwGL2_NewFrame();
  ImGui::EndFrame();

  ImGuiStyle& style = ImGui::GetStyle();
  
  // Style the GUI colors to a neutral greyscale with plenty of transaparency to concentrate on the image.
  // Change these RGB values to get any other tint.
  const float r = 1.0f;
  const float g = 1.0f;
  const float b = 1.0f;
  
  style.Colors[ImGuiCol_Text]                  = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
  style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
  style.Colors[ImGuiCol_WindowBg]              = ImVec4(r * 0.2f, g * 0.2f, b * 0.2f, 0.6f);
  style.Colors[ImGuiCol_ChildWindowBg]         = ImVec4(r * 0.2f, g * 0.2f, b * 0.2f, 1.0f);
  style.Colors[ImGuiCol_PopupBg]               = ImVec4(r * 0.2f, g * 0.2f, b * 0.2f, 1.0f);
  style.Colors[ImGuiCol_Border]                = ImVec4(r * 0.4f, g * 0.4f, b * 0.4f, 0.4f);
  style.Colors[ImGuiCol_BorderShadow]          = ImVec4(r * 0.0f, g * 0.0f, b * 0.0f, 0.4f);
  style.Colors[ImGuiCol_FrameBg]               = ImVec4(r * 0.4f, g * 0.4f, b * 0.4f, 0.4f);
  style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(r * 0.6f, g * 0.6f, b * 0.6f, 0.6f);
  style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 0.8f);
  style.Colors[ImGuiCol_TitleBg]               = ImVec4(r * 0.6f, g * 0.6f, b * 0.6f, 0.6f);
  style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(r * 0.2f, g * 0.2f, b * 0.2f, 0.2f);
  style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 0.8f);
  style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(r * 0.2f, g * 0.2f, b * 0.2f, 1.0f);
  style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(r * 0.2f, g * 0.2f, b * 0.2f, 0.2f);
  style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(r * 0.4f, g * 0.4f, b * 0.4f, 0.4f);
  style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(r * 0.6f, g * 0.6f, b * 0.6f, 0.6f);
  style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 0.8f);
  style.Colors[ImGuiCol_CheckMark]             = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 0.8f);
  style.Colors[ImGuiCol_SliderGrab]            = ImVec4(r * 0.4f, g * 0.4f, b * 0.4f, 0.4f);
  style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 0.8f);
  style.Colors[ImGuiCol_Button]                = ImVec4(r * 0.4f, g * 0.4f, b * 0.4f, 0.4f);
  style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(r * 0.6f, g * 0.6f, b * 0.6f, 0.6f);
  style.Colors[ImGuiCol_ButtonActive]          = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 0.8f);
  style.Colors[ImGuiCol_Header]                = ImVec4(r * 0.4f, g * 0.4f, b * 0.4f, 0.4f);
  style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(r * 0.6f, g * 0.6f, b * 0.6f, 0.6f);
  style.Colors[ImGuiCol_HeaderActive]          = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 0.8f);
  style.Colors[ImGuiCol_Column]                = ImVec4(r * 0.4f, g * 0.4f, b * 0.4f, 0.4f);
  style.Colors[ImGuiCol_ColumnHovered]         = ImVec4(r * 0.6f, g * 0.6f, b * 0.6f, 0.6f);
  style.Colors[ImGuiCol_ColumnActive]          = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 0.8f);
  style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(r * 0.6f, g * 0.6f, b * 0.6f, 0.6f);
  style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 0.8f);
  style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(r * 1.0f, g * 1.0f, b * 1.0f, 1.0f);
  style.Colors[ImGuiCol_CloseButton]           = ImVec4(r * 0.4f, g * 0.4f, b * 0.4f, 0.4f);
  style.Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(r * 0.6f, g * 0.6f, b * 0.6f, 0.6f);
  style.Colors[ImGuiCol_CloseButtonActive]     = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 0.8f);
  style.Colors[ImGuiCol_PlotLines]             = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 1.0f);
  style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(r * 1.0f, g * 1.0f, b * 1.0f, 1.0f);
  style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 1.0f);
  style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(r * 1.0f, g * 1.0f, b * 1.0f, 1.0f);
  style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(r * 0.5f, g * 0.5f, b * 0.5f, 1.0f);
  style.Colors[ImGuiCol_ModalWindowDarkening]  = ImVec4(r * 0.2f, g * 0.2f, b * 0.2f, 0.2f);
  style.Colors[ImGuiCol_DragDropTarget]        = ImVec4(r * 1.0f, g * 1.0f, b * 0.0f, 1.0f); // Yellow
  style.Colors[ImGuiCol_NavHighlight]          = ImVec4(r * 1.0f, g * 1.0f, b * 1.0f, 1.0f);
  style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(r * 1.0f, g * 1.0f, b * 1.0f, 1.0f);

  // Renderer setup and GUI parameters.
  m_minPathLength       = 2;    // Minimum path length after which Russian Roulette path termination starts.
  m_maxPathLength       = 2;    // Maximum path length. (Need at least 6 path segments to go through a glass sphere, hit something, and back through that sphere to the viewer.)
  m_sceneEpsilonFactor  = 500;  // Factor on 1e-7 used to offset ray origins along the path to reduce self intersections. 

  m_present         = false;  // Update once per second. (The first half second shows all frames to get some initial accumulation).
  m_presentNext     = true;
  m_presentAtSecond = 1.0;

  m_builder = std::string("Trbvh");
  
  m_frames = 0; // Samples per pixel. 0 == render forever.

  // GLSL shaders objects and program. 
  m_glslVS      = 0;
  m_glslFS      = 0;
  m_glslProgram = 0;

  // Settings normally used.
  //m_gamma          = 2.2f;
  //m_colorBalance   = optix::make_float3(1.0f);
  //m_whitePoint     = 1.0f;
  //m_burnHighlights = 0.8f;
  //m_crushBlacks    = 0.2f;
  //m_saturation     = 1.2f;
  //m_brightness     = 0.8f;

  // Neutral tonemapper settings.
  // This sample begins with an Ambient Occlusion like rendering setup. Make sure the the image stays white.
  m_gamma          = 2.2f; // Neutral would be 1.0f.
  m_colorBalance   = optix::make_float3(1.0f);
  m_whitePoint     = 1.0f;
  m_burnHighlights = 1.0f;
  m_crushBlacks    = 0.0f;
  m_saturation     = 1.0f;
  m_brightness     = 1.0f;

  m_guiState = GUI_STATE_NONE;

  m_isWindowVisible = true;

  m_mouseSpeedRatio = 10.0f;

  m_pinholeCamera.setViewport(m_width, m_height);

  initOpenGL();
  initOptiX(); // Sets m_isValid when OptiX initialization was successful.
}

Application::~Application()
{
  // DAR FIXME Do any other destruction here.
  if (m_isValid)
  {
    m_context->destroy();
  }

  ImGui_ImplGlfwGL2_Shutdown();
  ImGui::DestroyContext();
}

bool Application::isValid() const
{
  return m_isValid;
}

void Application::reshape(int width, int height)
{
  if ((width != 0 && height != 0) && // Zero sized interop buffers are not allowed in OptiX.
      (m_width != width || m_height != height))
  {
    m_width  = width;
    m_height = height;

    glViewport(0, 0, m_width, m_height);
    try
    {
      m_bufferOutput->setSize(m_width, m_height); // RGBA32F buffer.

      // When not using the deoiser this is the buffer which is displayed.
      if (m_interop)
      {
        m_bufferOutput->unregisterGLBuffer(); // Must unregister or CUDA won't notice the size change and crash.
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_bufferOutput->getGLBOId());
        glBufferData(GL_PIXEL_UNPACK_BUFFER, m_bufferOutput->getElementSize() * m_width * m_height, nullptr, GL_STREAM_DRAW);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        m_bufferOutput->registerGLBuffer();
      }
    }
    catch(optix::Exception& e)
    {
      std::cerr << e.getErrorString() << std::endl;
    }

    m_pinholeCamera.setViewport(m_width, m_height);

    restartAccumulation();
  }
}

void Application::guiNewFrame()
{
  ImGui_ImplGlfwGL2_NewFrame();
}

void Application::guiReferenceManual()
{
  ImGui::ShowTestWindow();
}

void Application::guiRender()
{
  ImGui::Render();
  ImGui_ImplGlfwGL2_RenderDrawData(ImGui::GetDrawData());
}


void Application::getSystemInformation()
{
  unsigned int optixVersion;
  RT_CHECK_ERROR_NO_CONTEXT(rtGetVersion(&optixVersion));

  unsigned int major = optixVersion / 1000; // Check major with old formula.
  unsigned int minor;
  unsigned int micro;
  if (3 < major) // New encoding since OptiX 4.0.0 to get two digits micro numbers?
  {
    major =  optixVersion / 10000;
    minor = (optixVersion % 10000) / 100;
    micro =  optixVersion % 100;
  }
  else // Old encoding with only one digit for the micro number.
  {
    minor = (optixVersion % 1000) / 10;
    micro =  optixVersion % 10;
  }
  std::cout << "OptiX " << major << "." << minor << "." << micro << std::endl;
  
  unsigned int numberOfDevices = 0;
  RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetDeviceCount(&numberOfDevices));
  std::cout << "Number of Devices = " << numberOfDevices << std::endl << std::endl;

  for (unsigned int i = 0; i < numberOfDevices; ++i)
  {
    char name[256];
    RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_NAME, sizeof(name), name));
    std::cout << "Device " << i << ": " << name << std::endl;
  
    int computeCapability[2] = {0, 0};
    RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY, sizeof(computeCapability), &computeCapability));
    std::cout << "  Compute Support: " << computeCapability[0] << "." << computeCapability[1] << std::endl;

    RTsize totalMemory = 0;
    RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_TOTAL_MEMORY, sizeof(totalMemory), &totalMemory));
    std::cout << "  Total Memory: " << (unsigned long long) totalMemory << std::endl;

    int clockRate = 0;
    RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_CLOCK_RATE, sizeof(clockRate), &clockRate));
    std::cout << "  Clock Rate: " << clockRate << " kHz" << std::endl;

    int maxThreadsPerBlock = 0;
    RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK, sizeof(maxThreadsPerBlock), &maxThreadsPerBlock));
    std::cout << "  Max. Threads per Block: " << maxThreadsPerBlock << std::endl;

    int smCount = 0;
    RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT, sizeof(smCount), &smCount));
    std::cout << "  Streaming Multiprocessor Count: " << smCount << std::endl;

    int executionTimeoutEnabled = 0;
    RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_EXECUTION_TIMEOUT_ENABLED, sizeof(executionTimeoutEnabled), &executionTimeoutEnabled));
    std::cout << "  Execution Timeout Enabled: " << executionTimeoutEnabled << std::endl;

    int maxHardwareTextureCount = 0 ;
    RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_MAX_HARDWARE_TEXTURE_COUNT, sizeof(maxHardwareTextureCount), &maxHardwareTextureCount));
    std::cout << "  Max. Hardware Texture Count: " << maxHardwareTextureCount << std::endl;
 
    int tccDriver = 0;
    RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_TCC_DRIVER, sizeof(tccDriver), &tccDriver));
    std::cout << "  TCC Driver enabled: " << tccDriver << std::endl;
 
    int cudaDeviceOrdinal = 0;
    RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_CUDA_DEVICE_ORDINAL, sizeof(cudaDeviceOrdinal), &cudaDeviceOrdinal));
    std::cout << "  CUDA Device Ordinal: " << cudaDeviceOrdinal << std::endl << std::endl;
  }
}

void Application::initOpenGL()
{
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  glViewport(0, 0, m_width, m_height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if (m_interop)
  {
    // PBO for the fast OptiX sysOutputBuffer to texture transfer.
    glGenBuffers(1, &m_pboOutputBuffer);
    MY_ASSERT(m_pboOutputBuffer != 0); 
    // Buffer size must be > 0 or OptiX can't create a buffer from it.
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboOutputBuffer);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, m_width * m_height * sizeof(float) * 4, (void*) 0, GL_STREAM_READ); // RGBA32F from byte offset 0 in the pixel unpack buffer.
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  }

  // glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // default, works for BGRA8, RGBA16F, and RGBA32F.

  glGenTextures(1, &m_hdrTexture);
  MY_ASSERT(m_hdrTexture != 0);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_hdrTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_2D, 0);

  // DAR ImGui has been changed to push the GL_TEXTURE_BIT so that this works. 
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  initGLSL();
}


void Application::initOptiX()
{
  try
  {
    getSystemInformation();

    m_context = optix::Context::create();

    // Select the GPUs to use with this context.
    unsigned int numberOfDevices = 0;
    RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetDeviceCount(&numberOfDevices));
    std::cout << "Number of Devices = " << numberOfDevices << std::endl << std::endl;

    std::vector<int> devices;

    int devicesEncoding = m_devicesEncoding; // Preserve this information, it can be stored in the system file.
    unsigned int i = 0;
    do 
    {
      int device = devicesEncoding % 10;
      devices.push_back(device); // DAR FIXME Should be a std::set to prevent duplicate device IDs in m_devicesEncoding.
      devicesEncoding /= 10;
      ++i;
    } while (i < numberOfDevices && devicesEncoding);

    m_context->setDevices(devices.begin(), devices.end());
    
    // Print out the current configuration to make sure what's currently running.
    devices = m_context->getEnabledDevices();
    for (size_t i = 0; i < devices.size(); ++i) 
    {
      std::cout << "m_context is using local device " << devices[i] << ": " << m_context->getDeviceName(devices[i]) << std::endl;
    }
    std::cout << "OpenGL interop is " << ((m_interop) ? "enabled" : "disabled") << std::endl;

    initPrograms();
    initRenderer(); 
    initScene();

    m_isValid = true; // If we get here with no exception, flag the initialization as successful. Otherwise the app will exit with error message.
  }
  catch(optix::Exception& e)
  {
    std::cerr << e.getErrorString() << std::endl;
  }
}

void Application::initRenderer() 
{
  try
  {
    m_context->setEntryPointCount(1); // 0 = render // Tonemapper is a GLSL shader in this case.
    m_context->setRayTypeCount(1);    // 0 = radiance
    
    m_context->setStackSize(m_stackSize);
    std::cout << "stackSize = " << m_stackSize << std::endl;

#if USE_DEBUG_EXCEPTIONS
    // Disable this by default for performance, otherwise the stitched PTX code will have lots of exception handling inside. 
    m_context->setPrintEnabled(true);
    //m_context->setPrintLaunchIndex(256, 256);
    m_context->setExceptionEnabled(RT_EXCEPTION_ALL, true);
#endif 

    // Add context-global variables here.
    m_context["sysSceneEpsilon"]->setFloat(m_sceneEpsilonFactor * 1e-7f);
    m_context["sysPathLengths"]->setInt(m_minPathLength, m_maxPathLength);
    m_context["sysIterationIndex"]->setInt(0); // With manual accumulation, 0 fills the buffer, accumulation starts at 1. On the VCA this variable is unused!
  
    // RT_BUFFER_INPUT_OUTPUT to support accumulation.
    // (In case of an OpenGL interop buffer, that is automatically registered with CUDA now! Must unregister/register around size changes.)
    m_bufferOutput = (m_interop) ? m_context->createBufferFromGLBO(RT_BUFFER_INPUT_OUTPUT, m_pboOutputBuffer)
                                 : m_context->createBuffer(RT_BUFFER_INPUT_OUTPUT);
    m_bufferOutput->setFormat(RT_FORMAT_FLOAT4); // RGBA32F
    m_bufferOutput->setSize(m_width, m_height);

    m_context["sysOutputBuffer"]->set(m_bufferOutput);

    std::map<std::string, optix::Program>::const_iterator it = m_mapOfPrograms.find("raygeneration");
    MY_ASSERT(it != m_mapOfPrograms.end()); 
    m_context->setRayGenerationProgram(0, it->second); // entrypoint

    it = m_mapOfPrograms.find("exception");
    MY_ASSERT(it != m_mapOfPrograms.end()); 
    m_context->setExceptionProgram(0, it->second); // entrypoint

    it = m_mapOfPrograms.find("miss");
    MY_ASSERT(it != m_mapOfPrograms.end()); 
    m_context->setMissProgram(0, it->second); // raytype

    // Default initialization. Will be overwritten on the first frame.
    m_context["sysCameraPosition"]->setFloat(0.0f, 0.0f, 1.0f);
    m_context["sysCameraU"]->setFloat(1.0f, 0.0f, 0.0f);
    m_context["sysCameraV"]->setFloat(0.0f, 1.0f, 0.0f);
    m_context["sysCameraW"]->setFloat(0.0f, 0.0f, -1.0f);
  }
  catch(optix::Exception& e)
  {
    std::cerr << e.getErrorString() << std::endl;
  }
}


void Application::initScene()
{
  try
  {
    m_timer.restart();
    const double timeInit = m_timer.getTime();

    std::cout << "createScene()" << std::endl;
    createScene();
    const double timeScene = m_timer.getTime();

    std::cout << "m_context->validate()" << std::endl;
    m_context->validate();
    const double timeValidate = m_timer.getTime();

    std::cout << "m_context->launch()" << std::endl;
    m_context->launch(0, 0, 0); // Dummy launch to build everything (entrypoint, width, height)
    const double timeLaunch = m_timer.getTime();

    std::cout << "initScene(): " << timeLaunch - timeInit << " seconds overall" << std::endl;
    std::cout << "{" << std::endl;
    std::cout << "  createScene() = " << timeScene    - timeInit     << " seconds" << std::endl;
    std::cout << "  validate()    = " << timeValidate - timeScene    << " seconds" << std::endl;
    std::cout << "  launch()      = " << timeLaunch   - timeValidate << " seconds" << std::endl;
    std::cout << "}" << std::endl;
  }
  catch(optix::Exception& e)
  {
    std::cerr << e.getErrorString() << std::endl;
  }
}

void Application::restartAccumulation()
{
  m_iterationIndex  = 0;
  m_presentNext     = true;
  m_presentAtSecond = 1.0;

  m_timer.restart();
}


bool Application::render()
{
  bool repaint = false;

  try
  {
    optix::float3 cameraPosition;
    optix::float3 cameraU;
    optix::float3 cameraV;
    optix::float3 cameraW;

    bool cameraChanged = m_pinholeCamera.getFrustum(cameraPosition, cameraU, cameraV, cameraW);
    if (cameraChanged)
    {
      m_context["sysCameraPosition"]->setFloat(cameraPosition);
      m_context["sysCameraU"]->setFloat(cameraU);
      m_context["sysCameraV"]->setFloat(cameraV);
      m_context["sysCameraW"]->setFloat(cameraW);

      restartAccumulation();
    }
  
    // Continue manual accumulation rendering if there is no limit (m_frames == 0) or the number of frames has not been reached.
    if (0 == m_frames || m_iterationIndex < m_frames)
    {
      m_context["sysIterationIndex"]->setInt(m_iterationIndex); // Iteration index is zero-based!
      m_context->launch(0, m_width, m_height);
      m_iterationIndex++;
    }

    // Only update the texture when a restart happened or one second passed to reduce required bandwidth.
    if (m_presentNext)
    {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, m_hdrTexture); // Manual accumulation always renders into the m_hdrTexture.

      if (m_interop) 
      {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_bufferOutput->getGLBOId());
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (GLsizei) m_width, (GLsizei) m_height, 0, GL_RGBA, GL_FLOAT, (void*) 0); // RGBA32F from byte offset 0 in the pixel unpack buffer.
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
      }
      else
      {
        const void* data = m_bufferOutput->map(0, RT_BUFFER_MAP_READ);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (GLsizei) m_width, (GLsizei) m_height, 0, GL_RGBA, GL_FLOAT, data); // RGBA32F
        m_bufferOutput->unmap();
      }

      repaint = true; // Indicate that there is a new image.

      m_presentNext = m_present;
    }

    double seconds = m_timer.getTime();
#if 1
    // Show the accumulation of the first half second to remain interactive with "present 0" on the VCA.
    // Not done when benchmarking to get more accurate results.
    if (seconds < 0.5)
    {
      m_presentAtSecond = 1.0;
      m_presentNext     = true;
    }
    else 
#endif
    if (m_presentAtSecond < seconds)
    {
      m_presentAtSecond = ceil(seconds);
      
      const double fps = double(m_iterationIndex) / seconds;

      std::ostringstream stream; 
      stream.precision(3); // Precision is # digits in fraction part.
      // m_iterationIndex has already been incremented for the last rendered frame, so it is the actual framecount here.
      stream << std::fixed << m_iterationIndex << " / " << seconds << " = " << fps << " fps";
      std::cout << stream.str() << std::endl;

      m_presentNext = true; // Present at least every second.
    }
  }
  catch(optix::Exception& e)
  {
    std::cerr << e.getErrorString() << std::endl;
  }
  return repaint;
}

void Application::display()
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_hdrTexture);

  glUseProgram(m_glslProgram);

  glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
  glEnd();

  glUseProgram(0);
}

void Application::screenshot(std::string const& filename)
{
  sutil::writeBufferToFile(filename.c_str(), m_bufferOutput);
  std::cerr << "Wrote " << filename << std::endl;
}

// Helper functions:
void Application::checkInfoLog(const char *msg, GLuint object)
{
  GLint maxLength;
  GLint length;
  GLchar *infoLog;

  if (glIsProgram(object))
  {
    glGetProgramiv(object, GL_INFO_LOG_LENGTH, &maxLength);
  }
  else
  {
    glGetShaderiv(object, GL_INFO_LOG_LENGTH, &maxLength);
  }
  if (maxLength > 1) 
  {
    infoLog = (GLchar *) malloc(maxLength);
    if (infoLog != NULL)
    {
      if (glIsShader(object))
      {
        glGetShaderInfoLog(object, maxLength, &length, infoLog);
      }
      else
      {
        glGetProgramInfoLog(object, maxLength, &length, infoLog);
      }
      //fprintf(fileLog, "-- tried to compile (len=%d): %s\n", (unsigned int)strlen(msg), msg);
      //fprintf(fileLog, "--- info log contents (len=%d) ---\n", (int) maxLength);
      //fprintf(fileLog, "%s", infoLog);
      //fprintf(fileLog, "--- end ---\n");
      std::cout << infoLog << std::endl;
      // Look at the info log string here...
      free(infoLog);
    }
  }
}


void Application::initGLSL()
{
  static const std::string vsSource =
    "#version 330\n"
    "layout(location = 0) in vec4 attrPosition;\n"
    "layout(location = 8) in vec2 attrTexCoord0;\n"
    "out vec2 varTexCoord0;\n"
    "void main()\n"
    "{\n"
    "  gl_Position  = attrPosition;\n"
    "  varTexCoord0 = attrTexCoord0;\n"
    "}\n";

  static const std::string fsSource =
    "#version 330\n"
    "uniform sampler2D samplerHDR;\n"
    "uniform vec3  colorBalance;\n"
    "uniform float invWhitePoint;\n"
    "uniform float burnHighlights;\n"
    "uniform float saturation;\n"
    "uniform float crushBlacks;\n"
    "uniform float invGamma;\n"
    "in vec2 varTexCoord0;\n"
    "layout(location = 0, index = 0) out vec4 outColor;\n"
    "void main()\n"
    "{\n"
    "  vec3 hdrColor = texture(samplerHDR, varTexCoord0).rgb;\n"
    "  vec3 ldrColor = invWhitePoint * colorBalance * hdrColor;\n"
    "  ldrColor *= (ldrColor * burnHighlights + 1.0) / (ldrColor + 1.0);\n"
    "  float luminance = dot(ldrColor, vec3(0.3, 0.59, 0.11));\n"
    "  ldrColor = max(mix(vec3(luminance), ldrColor, saturation), 0.0);\n"
    "  luminance = dot(ldrColor, vec3(0.3, 0.59, 0.11));\n"
    "  if (luminance < 1.0)\n"
    "  {\n"
    "    ldrColor = max(mix(pow(ldrColor, vec3(crushBlacks)), ldrColor, sqrt(luminance)), 0.0);\n"
    "  }\n"
    "  ldrColor = pow(ldrColor, vec3(invGamma));\n"
    "  outColor = vec4(ldrColor, 1.0);\n"
    "}\n";

  GLint vsCompiled = 0;
  GLint fsCompiled = 0;
    
  m_glslVS = glCreateShader(GL_VERTEX_SHADER);
  if (m_glslVS)
  {
    GLsizei len = (GLsizei) vsSource.size();
    const GLchar *vs = vsSource.c_str();
    glShaderSource(m_glslVS, 1, &vs, &len);
    glCompileShader(m_glslVS);
    checkInfoLog(vs, m_glslVS);

    glGetShaderiv(m_glslVS, GL_COMPILE_STATUS, &vsCompiled);
    MY_ASSERT(vsCompiled);
  }

  m_glslFS = glCreateShader(GL_FRAGMENT_SHADER);
  if (m_glslFS)
  {
    GLsizei len = (GLsizei) fsSource.size();
    const GLchar *fs = fsSource.c_str();
    glShaderSource(m_glslFS, 1, &fs, &len);
    glCompileShader(m_glslFS);
    checkInfoLog(fs, m_glslFS);

    glGetShaderiv(m_glslFS, GL_COMPILE_STATUS, &fsCompiled);
    MY_ASSERT(fsCompiled);
  }

  m_glslProgram = glCreateProgram();
  if (m_glslProgram)
  {
    GLint programLinked = 0;

    if (m_glslVS && vsCompiled)
    {
      glAttachShader(m_glslProgram, m_glslVS);
    }
    if (m_glslFS && fsCompiled)
    {
      glAttachShader(m_glslProgram, m_glslFS);
    }

    glLinkProgram(m_glslProgram);
    checkInfoLog("m_glslProgram", m_glslProgram);

    glGetProgramiv(m_glslProgram, GL_LINK_STATUS, &programLinked);
    MY_ASSERT(programLinked);

    if (programLinked)
    {
      glUseProgram(m_glslProgram);
      
      glUniform1i(glGetUniformLocation(m_glslProgram, "samplerHDR"), 0);       // texture image unit 0
      glUniform1f(glGetUniformLocation(m_glslProgram, "invGamma"), 1.0f / m_gamma);
      glUniform3f(glGetUniformLocation(m_glslProgram, "colorBalance"), m_colorBalance.x, m_colorBalance.y, m_colorBalance.z);
      glUniform1f(glGetUniformLocation(m_glslProgram, "invWhitePoint"), m_brightness / m_whitePoint);
      glUniform1f(glGetUniformLocation(m_glslProgram, "burnHighlights"), m_burnHighlights);
      glUniform1f(glGetUniformLocation(m_glslProgram, "crushBlacks"), m_crushBlacks + m_crushBlacks + 1.0f);
      glUniform1f(glGetUniformLocation(m_glslProgram, "saturation"), m_saturation);

      glUseProgram(0);
    }
  }
}


void Application::guiWindow()
{
  if (!m_isWindowVisible) // Use SPACE to toggle the display of the GUI window.
  {
    return;
  }

  ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiSetCond_FirstUseEver);

  ImGuiWindowFlags window_flags = 0;
  if (!ImGui::Begin("optixIntro_04", nullptr, window_flags)) // No bool flag to omit the close button.
  {
    // Early out if the window is collapsed, as an optimization.
    ImGui::End();
    return;
  }

  ImGui::PushItemWidth(-100); // right-aligned, keep 180 pixels for the labels.

  if (ImGui::CollapsingHeader("System"))
  {
    if (ImGui::Checkbox("Present", &m_present))
    {
      // No action needed, happens automatically.
    }
    if (ImGui::DragInt("Min Paths", &m_minPathLength, 1.0f, 0, 100))
    {
      m_context["sysPathLengths"]->setInt(m_minPathLength, m_maxPathLength);
      restartAccumulation();
    }
    if (ImGui::DragInt("Max Paths", &m_maxPathLength, 1.0f, 0, 100))
    {
      m_context["sysPathLengths"]->setInt(m_minPathLength, m_maxPathLength);
      restartAccumulation();
    }
    if (ImGui::DragFloat("Scene Epsilon", &m_sceneEpsilonFactor, 1.0f, 0.0f, 10000.0f))
    {
      m_context["sysSceneEpsilon"]->setFloat(m_sceneEpsilonFactor * 1e-7f);
      restartAccumulation();
    }
    if (ImGui::DragInt("Frames", &m_frames, 1.0f, 0, 10000))
    {
      if (m_frames != 0 && m_frames < m_iterationIndex) // If we already rendered more frames, start again.
      {
        restartAccumulation();
      }
    }
    if (ImGui::DragFloat("Mouse Ratio", &m_mouseSpeedRatio, 0.1f, 0.1f, 100.0f, "%.1f"))
    {
      m_pinholeCamera.setSpeedRatio(m_mouseSpeedRatio);
    }
  }
  if (ImGui::CollapsingHeader("Tonemapper"))
  {
    if (ImGui::ColorEdit3("Balance", (float*) &m_colorBalance))
    {
      glUseProgram(m_glslProgram);
      glUniform3f(glGetUniformLocation(m_glslProgram, "colorBalance"), m_colorBalance.x, m_colorBalance.y, m_colorBalance.z);
      glUseProgram(0);
    }
    if (ImGui::DragFloat("Gamma", &m_gamma, 0.01f, 0.01f, 10.0f)) // Must not get 0.0f
    {
      glUseProgram(m_glslProgram);
      glUniform1f(glGetUniformLocation(m_glslProgram, "invGamma"), 1.0f / m_gamma);
      glUseProgram(0);
    }
    if (ImGui::DragFloat("White Point", &m_whitePoint, 0.01f, 0.01f, 255.0f, "%.2f", 2.0f)) // Must not get 0.0f
    {
      glUseProgram(m_glslProgram);
      glUniform1f(glGetUniformLocation(m_glslProgram, "invWhitePoint"), m_brightness / m_whitePoint);
      glUseProgram(0);
    }
    if (ImGui::DragFloat("Burn Lights", &m_burnHighlights, 0.01f, 0.0f, 10.0f, "%.2f"))
    {
      glUseProgram(m_glslProgram);
      glUniform1f(glGetUniformLocation(m_glslProgram, "burnHighlights"), m_burnHighlights);
      glUseProgram(0);
    }
    if (ImGui::DragFloat("Crush Blacks", &m_crushBlacks, 0.01f, 0.0f, 1.0f, "%.2f"))
    {
      glUseProgram(m_glslProgram);
      glUniform1f(glGetUniformLocation(m_glslProgram, "crushBlacks"),  m_crushBlacks + m_crushBlacks + 1.0f);
      glUseProgram(0);
    }
    if (ImGui::DragFloat("Saturation", &m_saturation, 0.01f, 0.0f, 10.0f, "%.2f"))
    {
      glUseProgram(m_glslProgram);
      glUniform1f(glGetUniformLocation(m_glslProgram, "saturation"), m_saturation);
      glUseProgram(0);
    }
    if (ImGui::DragFloat("Brightness", &m_brightness, 0.01f, 0.0f, 100.0f, "%.2f", 2.0f))
    {
      glUseProgram(m_glslProgram);
      glUniform1f(glGetUniformLocation(m_glslProgram, "invWhitePoint"), m_brightness / m_whitePoint);
      glUseProgram(0);
    }
  }
  if (ImGui::CollapsingHeader("Materials"))
  {
    bool changed = false;

    for (int i = 0; i < int(m_guiMaterialParameters.size()); ++i)
    {
      if (ImGui::TreeNode((void*)(intptr_t) i, "Material %d", i))
      {
        MaterialParameterGUI& parameters = m_guiMaterialParameters[i];

        if (ImGui::ColorEdit3("Albedo", (float*) &parameters.albedo))
        {
          changed = true;
        }
        ImGui::TreePop();
      }
    }
    
    if (changed) // If any of the material parameters changed, simply upload them to the sysMaterialParameters again.
    {
      updateMaterialParameters();
      restartAccumulation();
    }
  }

  ImGui::PopItemWidth();

  ImGui::End();
}

void Application::guiEventHandler()
{
  ImGuiIO const& io = ImGui::GetIO();

  if (ImGui::IsKeyPressed(' ', false)) // Toggle the GUI window display with SPACE key.
  {
    m_isWindowVisible = !m_isWindowVisible;
  }

  const ImVec2 mousePosition = ImGui::GetMousePos(); // Mouse coordinate window client rect.
  const int x = int(mousePosition.x);
  const int y = int(mousePosition.y);

  switch (m_guiState)
  {
    case GUI_STATE_NONE:
      if (!io.WantCaptureMouse) // Only allow camera interactions to begin when not interacting with the GUI.
      {
        if (ImGui::IsMouseDown(0)) // LMB down event?
        {
          m_pinholeCamera.setBaseCoordinates(x, y);
          m_guiState = GUI_STATE_ORBIT;
        }
        else if (ImGui::IsMouseDown(1)) // RMB down event?
        {
          m_pinholeCamera.setBaseCoordinates(x, y);
          m_guiState = GUI_STATE_DOLLY;
        }
        else if (ImGui::IsMouseDown(2)) // MMB down event?
        {
          m_pinholeCamera.setBaseCoordinates(x, y);
          m_guiState = GUI_STATE_PAN;
        }
        else if (io.MouseWheel != 0.0f) // Mouse wheel zoom.
        {
          m_pinholeCamera.zoom(io.MouseWheel);
        }
      }
      break;

    case GUI_STATE_ORBIT:
      if (ImGui::IsMouseReleased(0)) // LMB released? End of orbit mode.
      {
        m_guiState = GUI_STATE_NONE;
      }
      else
      {
        m_pinholeCamera.orbit(x, y);
      }
      break;

    case GUI_STATE_DOLLY:
      if (ImGui::IsMouseReleased(1)) // RMB released? End of dolly mode.
      {
        m_guiState = GUI_STATE_NONE;
      }
      else
      {
        m_pinholeCamera.dolly(x, y);
      }
      break;

    case GUI_STATE_PAN:
      if (ImGui::IsMouseReleased(2)) // MMB released? End of pan mode.
      {
        m_guiState = GUI_STATE_NONE;
      }
      else
      {
        m_pinholeCamera.pan(x, y);
      }
      break;
  }
}


// This part is always identical in the generated geometry creation routines.
optix::Geometry Application::createGeometry(std::vector<VertexAttributes> const& attributes, std::vector<unsigned int> const& indices)
{
  optix::Geometry geometry(nullptr);

  try
  {
    geometry = m_context->createGeometry();

    optix::Buffer attributesBuffer = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_USER);
    attributesBuffer->setElementSize(sizeof(VertexAttributes));
    attributesBuffer->setSize(attributes.size());

    void *dst = attributesBuffer->map(0, RT_BUFFER_MAP_WRITE_DISCARD);
    memcpy(dst, attributes.data(), sizeof(VertexAttributes) * attributes.size());
    attributesBuffer->unmap();

    optix::Buffer indicesBuffer = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_UNSIGNED_INT3, indices.size() / 3);
    dst = indicesBuffer->map(0, RT_BUFFER_MAP_WRITE_DISCARD);
    memcpy(dst, indices.data(), sizeof(optix::uint3) * indices.size() / 3);
    indicesBuffer->unmap();

    std::map<std::string, optix::Program>::const_iterator it = m_mapOfPrograms.find("boundingbox_triangle_indexed");
    MY_ASSERT(it != m_mapOfPrograms.end()); 
    geometry->setBoundingBoxProgram(it->second);

    it = m_mapOfPrograms.find("intersection_triangle_indexed");
    MY_ASSERT(it != m_mapOfPrograms.end()); 
    geometry->setIntersectionProgram(it->second);

    geometry["attributesBuffer"]->setBuffer(attributesBuffer);
    geometry["indicesBuffer"]->setBuffer(indicesBuffer);
    geometry->setPrimitiveCount((unsigned int)(indices.size()) / 3);
  }
  catch(optix::Exception& e)
  {
    std::cerr << e.getErrorString() << std::endl;
  }
  return geometry;
}


void Application::initPrograms()
{
  try
  {
    // First load all programs and put them into a map.
    // Programs which are reused multiple times can be queried from that map.
    // (This renderer does not put variables on program scope!)

    // Renderer
    m_mapOfPrograms["raygeneration"] = m_context->createProgramFromPTXFile(ptxPath("raygeneration.cu"), "raygeneration"); // entry point 0
    m_mapOfPrograms["exception"]     = m_context->createProgramFromPTXFile(ptxPath("exception.cu"), "exception"); // entry point 0

    m_mapOfPrograms["miss"] = m_context->createProgramFromPTXFile(ptxPath("miss.cu"), "miss_environment_constant"); // raytype 0

    // Geometry
    m_mapOfPrograms["boundingbox_triangle_indexed"]  = m_context->createProgramFromPTXFile(ptxPath("boundingbox_triangle_indexed.cu"),  "boundingbox_triangle_indexed");
    m_mapOfPrograms["intersection_triangle_indexed"] = m_context->createProgramFromPTXFile(ptxPath("intersection_triangle_indexed.cu"), "intersection_triangle_indexed");

    // Material programs. There are only three Material nodes, opaque, cutout opacity and rectangle lights.
    // For the radiance ray type 0:
    m_mapOfPrograms["closesthit"] = m_context->createProgramFromPTXFile(ptxPath("closesthit.cu"), "closesthit");
  }
  catch(optix::Exception& e)
  {
    std::cerr << e.getErrorString() << std::endl;
  }
}

void Application::updateMaterialParameters()
{
  MY_ASSERT((sizeof(MaterialParameter) & 15) == 0); // Verify float4 alignment.

  // Convert the GUI material parameters to the device side structure and upload them into the context global buffer.
  // (Doing this in a loop will make more sense in later examples.)
  MaterialParameter* dst = static_cast<MaterialParameter*>(m_bufferMaterialParameters->map(0, RT_BUFFER_MAP_WRITE_DISCARD));

  for (size_t i = 0; i < m_guiMaterialParameters.size(); ++i, ++dst)
  {
    MaterialParameterGUI& src = m_guiMaterialParameters[i];
        
    dst->albedo = src.albedo;
  }

  m_bufferMaterialParameters->unmap();
}

void Application::initMaterials()
{
  // Setup GUI material parameters, one for each of the objects in the scene.
  MaterialParameterGUI parameters;

  // Make all parameters white to show automatic ambient occlusion with a brute force full global illumination path tracer.
  parameters.albedo = optix::make_float3(1.0f);
  m_guiMaterialParameters.push_back(parameters); // 0, floor

  parameters.albedo = optix::make_float3(1.0f);
  m_guiMaterialParameters.push_back(parameters); // 1, box

  parameters.albedo = optix::make_float3(1.0f);
  m_guiMaterialParameters.push_back(parameters); // 2, sphere

  parameters.albedo = optix::make_float3(1.0f);
  m_guiMaterialParameters.push_back(parameters); // 3, torus

  try
  {
    m_bufferMaterialParameters = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_USER);
    m_bufferMaterialParameters->setElementSize(sizeof(MaterialParameter));
    m_bufferMaterialParameters->setSize(m_guiMaterialParameters.size()); // As many as there are in the GUI.

    updateMaterialParameters();

    m_context["sysMaterialParameters"]->setBuffer(m_bufferMaterialParameters);

    // Create the main Material node to have the matching closest hit and any hit programs.

    std::map<std::string, optix::Program>::const_iterator it;

    // Used for all objects in the scene.
    m_opaqueMaterial = m_context->createMaterial();
    it = m_mapOfPrograms.find("closesthit");
    MY_ASSERT(it != m_mapOfPrograms.end());
    m_opaqueMaterial->setClosestHitProgram(0, it->second); // raytype radiance
  }
  catch(optix::Exception& e)
  {
    std::cerr << e.getErrorString() << std::endl;
  }
}


// Scene testing all materials on a single geometry instanced via transforms and sharing one acceleration structure.
void Application::createScene()
{
  initMaterials();

  try
  {
    // OptiX Scene Graph construction.
    m_rootAcceleration = m_context->createAcceleration(m_builder); // No need to set acceleration properties on the top level Acceleration.

    m_rootGroup = m_context->createGroup(); // The scene's root group nodes becomes the sysTopObject.
    m_rootGroup->setAcceleration(m_rootAcceleration);
    
    m_context["sysTopObject"]->set(m_rootGroup); // This is where the rtTrace calls start the BVH traversal. (Same for radiance and shadow rays.)
    
    unsigned int count;

    // Demo code only!
    // Mind that these local OptiX objects will leak when not cleaning up the scene properly on changes.
    // Destroying the OptiX context will clean them up at program exit though.

    // Add a ground plane on the xz-plane at y = 0.0f.
    optix::Geometry geoPlane = createPlane(1, 1, 1);

    optix::GeometryInstance giPlane = m_context->createGeometryInstance(); // This connects Geometries with Materials.
    giPlane->setGeometry(geoPlane);
    giPlane->setMaterialCount(1);
    giPlane->setMaterial(0, m_opaqueMaterial);
    giPlane["parMaterialIndex"]->setInt(0); // This is all! This defines which material parameters in sysMaterialParametrers to use.

    optix::Acceleration accPlane = m_context->createAcceleration(m_builder);
    setAccelerationProperties(accPlane);
    
    optix::GeometryGroup ggPlane = m_context->createGeometryGroup(); // This connects GeometryInstances with Acceleration structures. (All OptiX nodes with "Group" in the name hold an Acceleration.)
    ggPlane->setAcceleration(accPlane);
    ggPlane->setChildCount(1);
    ggPlane->setChild(0, giPlane);

    // Scale the plane to go from -8 to 8.
    float trafoPlane[16] =
    {
      8.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 8.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 8.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };
    optix::Matrix4x4 matrixPlane(trafoPlane);

    optix::Transform trPlane = m_context->createTransform();
    trPlane->setChild(ggPlane);
    trPlane->setMatrix(false, matrixPlane.getData(), matrixPlane.inverse().getData());
    
    count = m_rootGroup->getChildCount();
    m_rootGroup->setChildCount(count + 1);
    m_rootGroup->setChild(count, trPlane);

    // Add a box (no tessellation here, using just 12 triangles)
    optix::Geometry geoBox = createBox();

    optix::GeometryInstance giBox = m_context->createGeometryInstance();
    giBox->setGeometry(geoBox);
    giBox->setMaterialCount(1);
    giBox->setMaterial(0, m_opaqueMaterial);
    giBox["parMaterialIndex"]->setInt(1); // Using parameters in sysMaterialParameters[1].

    optix::Acceleration accBox = m_context->createAcceleration(m_builder);
    setAccelerationProperties(accBox);
    
    optix::GeometryGroup ggBox = m_context->createGeometryGroup();
    ggBox->setAcceleration(accBox);
    ggBox->setChildCount(1);
    ggBox->setChild(0, giBox);

    float trafoBox[16] =
    {
      1.0f, 0.0f, 0.0f, -2.5f, // Move to the left.
      0.0f, 1.0f, 0.0f, 1.25f, // The box is modeled with unit coordinates in the range [-1, 1], Move it above the floor plane.
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };
    optix::Matrix4x4 matrixBox(trafoBox);

    optix::Transform trBox = m_context->createTransform();
    trBox->setChild(ggBox);
    trBox->setMatrix(false, matrixBox.getData(), matrixBox.inverse().getData());
    
    count = m_rootGroup->getChildCount();
    m_rootGroup->setChildCount(count + 1);
    m_rootGroup->setChild(count, trBox);

    // Add a tessellated sphere with 180 longitudes and 90 latitudes, radius 1.0f and fully closed at the upper pole.
    optix::Geometry geoSphere = createSphere(180, 90, 1.0f, M_PIf);

    optix::GeometryInstance giSphere = m_context->createGeometryInstance();
    giSphere->setGeometry(geoSphere);
    giSphere->setMaterialCount(1);
    giSphere->setMaterial(0, m_opaqueMaterial);
    giSphere["parMaterialIndex"]->setInt(2); // Using parameters in sysMaterialParameters[2].

    optix::Acceleration accSphere = m_context->createAcceleration(m_builder);
    setAccelerationProperties(accSphere);
    
    optix::GeometryGroup ggSphere = m_context->createGeometryGroup();
    ggSphere->setAcceleration(accSphere);
    ggSphere->setChildCount(1);
    ggSphere->setChild(0, giSphere);

    float trafoSphere[16] =
    {
      1.0f, 0.0f, 0.0f, 0.0f,  // In the center, to the right of the box.
      0.0f, 1.0f, 0.0f, 1.25f, // The sphere is modeled with radius 1.0f. Move it above the floor plane to show shadows.
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };
    optix::Matrix4x4 matrixSphere(trafoSphere);

    optix::Transform trSphere = m_context->createTransform();
    trSphere->setChild(ggSphere);
    trSphere->setMatrix(false, matrixSphere.getData(), matrixSphere.inverse().getData());
    
    count = m_rootGroup->getChildCount();
    m_rootGroup->setChildCount(count + 1);
    m_rootGroup->setChild(count, trSphere);

    // Add a torus. 
    optix::Geometry geoTorus = createTorus(180, 180, 0.75f, 0.25f);

    optix::GeometryInstance giTorus = m_context->createGeometryInstance();
    giTorus->setGeometry(geoTorus);
    giTorus->setMaterialCount(1);
    giTorus->setMaterial(0, m_opaqueMaterial);
    giTorus["parMaterialIndex"]->setInt(3); // Using parameters in sysMaterialParameters[3].

    optix::Acceleration accTorus = m_context->createAcceleration(m_builder);
    setAccelerationProperties(accTorus);
    
    optix::GeometryGroup ggTorus = m_context->createGeometryGroup();
    ggTorus->setAcceleration(accTorus);
    ggTorus->setChildCount(1);
    ggTorus->setChild(0, giTorus);

    float trafoTorus[16] =
    {
      1.0f, 0.0f, 0.0f, 2.5f,  // Move it to the right of the sphere.
      0.0f, 1.0f, 0.0f, 1.25f, // The torus has an outer radius of 0.5f. Move it above the floor plane.
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };
    optix::Matrix4x4 matrixTorus(trafoTorus);

    optix::Transform trTorus = m_context->createTransform();
    trTorus->setChild(ggTorus);
    trTorus->setMatrix(false, matrixTorus.getData(), matrixTorus.inverse().getData());
    
    count = m_rootGroup->getChildCount();
    m_rootGroup->setChildCount(count + 1);
    m_rootGroup->setChild(count, trTorus);
  }
  catch(optix::Exception& e)
  {
    std::cerr << e.getErrorString() << std::endl;
  }
}


void Application::setAccelerationProperties(optix::Acceleration acceleration)
{
  // To speed up the acceleration structure build for triangles, skip calls to the bounding box program and
  // invoke the special splitting BVH builder for indexed triangles by setting the necessary acceleration properties.
  // Using the fast Trbvh builder which does splitting has a positive effect on the rendering performanc as well.
  if (m_builder == std::string("Trbvh") || m_builder == std::string("Sbvh"))
  {
    // This requires that the position is the first element and it must be float x, y, z.
    acceleration->setProperty("vertex_buffer_name", "attributesBuffer");
    MY_ASSERT(sizeof(VertexAttributes) == 48) ;
    acceleration->setProperty("vertex_buffer_stride", "48");

    acceleration->setProperty("index_buffer_name", "indicesBuffer");
    MY_ASSERT(sizeof(optix::uint3) == 12) ;
    acceleration->setProperty("index_buffer_stride", "12");
  }
}
