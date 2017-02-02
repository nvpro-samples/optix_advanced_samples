
optix_advanced_samples
==================

This is a set of samples for the [NVIDIA OptiX Ray Tracing Engine](https://developer.nvidia.com).  They are more advanced
than the basic tutorial-style samples that ship with the SDK.
Some, like optixOcean, are based on samples that used to ship with OptiX prior to
version 4, and some like optixVox are new samples.

#### Requirements
  * [CUDA Toolkit](https://developer.nvidia.com/cuda-downloads) version 7.5+ and matching driver with supported GPU.
  * [OptiX](https://developer.nvidia.com) version 4.0+
  * A recent version of Visual Studio or gcc.  We tested with VS 2013 on Windows 10 and gcc 4.8.4 on Ubuntu 14.04.
  * A recent version of [CMake](https://cmake.org).

Other support libraries, e.g, GLFW and imgui, are bundled with this repository.

#### How to Build & Run

  * Download and install the requirements above (CUDA, OptiX, CMake)

  * Clone this repository.  Samples are in optix_advanced_samples/SDK.

Follow the build instructions in [SDK/INSTALL-LINUX.txt](./SDK/INSTALL-LINUX.txt) or [SDK/INSTALL-WIN.txt](./SDK/INSTALL-WIN.txt).


