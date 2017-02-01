
optix_advanced_samples
==================

This is a set of samples for the [NVIDIA OptiX Ray Tracing Engine](https://developer.nvidia.com).  They are more advanced
than the basic tutorial-style samples that ship with the SDK.
Some, like optixOcean, are based on samples that used to ship with OptiX prior to
version 4, and some like optixVox are new samples.

#### Requirements
  * [OptiX](https://developer.nvidia.com) version 4.0 or greater
  * A recent version of Visual Studio or gcc.  We tested with VS 2013 on Windows 10 and gcc 4.8.4 on Ubuntu 14.04.
  * CUDA 7.5+ and matching driver with supported GPU.
  * A recent version of CMake.  We tested with 2.8.12.

Other support libraries, e.g, GLFW and imgui, are bundled with this repository.

#### How to Build & Run

Quick build instructions:

  * Download and install OptiX and CMake

  * Clone this repository

  * Open CMake-gui (Windows) or ccmake (Linux):
  - Source code to the SDK directory of the respository
  - Build folder: SDK/build
  - Configure and select a compiler if prompted.
  - Generate

  * Open the OptiX-Advanced-Samples.sln in Visual Studio, and Build All.  On Linux, 'make' in the build directory.

  * Select any sample as the Startup project in VS.  (Try optixHello as a smoke test).

  * Click Run in VS, or run the binary, e.g., ./bin/optixHello, in Linux.


Check the README files in the sample subdirectories.  To print a list of options for each sample, use "-h" on the shell.

