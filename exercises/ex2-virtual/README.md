# Exercise2 Virtual Sensor
## Goal
Implement a virtual device on OpenBMC and expose its data via D-Bus using sdbusplus.

## Investigation Steps
- I initially tried writing a script directly in QEMU to publish the service, but later found that I couldn't requestName, so it failed to register on D-Bus.
[v_ambient.sh](code/v_ambient.sh)

- Next, I tried using sdbusplus: I packaged the SDK on a Ubuntu VM, cross-compiled the C++ files using the Romulus environment, and then scp'd the binary file into QEMU
  1. For the SDK, please refer to the OpenBMC wiki
     ```
     source environment-setup-arm1176jzs-openbmc-linux-gnueabi
     ```
  2. Then, use CMake to compile the cpp file
     - [CMakeLists.txt](../../meta-henry/recipes-vambient/v-ambient/files/CMakeLists.txt)
     - [v_ambient_draft.cpp](code/v_ambient_draft.cpp)
  3. 
