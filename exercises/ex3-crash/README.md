# Exercise3 Crash Service Debug
## Goal
Simulate a crashing service on OpenBMC and perform full debugging flow:
- Trigger SIGSEGV
- Generate core dump
- Analyze with cross GDB

## Crash Program
A simple [C++ program](../../meta-henry/recipes-debugcrash/debugcrash/files/debugcrash.cpp) that dereferences a null pointer:
```cpp
int* ptr = nullptr;
std::cout << *ptr << std::endl
```

## Implementation
- To enable line number visibility in GDB, I explicitly added the -g debug flag in [CMakeLists.txt](../../meta-henry/recipes-debugcrash/debugcrash/files/CMakeLists.txt)
  ```
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
  ```

- I added the following options to [debugcrash.bb](../../meta-henry/recipes-debugcrash/debugcrash/debugcrash_git.bb) to prevent BitBake from stripping away the debug symbols
  ```
  INHIBIT_PACKAGE_STRIP = "1"
  INHIBIT_SYSROOT_STRIP = "1"
  INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
  ```

- To enable Core Dump generation and storage for Romulus, add the following configurations to [local.conf](../../build/romulus/conf/local.conf)
  ```
  IMAGE_FEATURES += "tools-debug"
  PACKAGECONFIG:append:pn-systemd = " coredump"
  ```

### Problem Encountered
1. Image size overflow:
   - tools-debug introduces large packages (gdb, strace)
   - squashfs image overflow
   ```
   	ERROR: obmc-phosphor-image-1.0-r0 do_generate_static: Image '/home/henry/openbmc/build/romulus/tmp/work/romulus-openbmc-linux-gnueabi/obmc-phosphor-image/1.0/deploy-obmc-phosphor-image-image-complete/obmc-phosphor-image-romulus.squashfs-xz' is 9474048 bytes too large!
   ```
2. Core dump not generated
   - `coredumpctl` shows: SIGSEGV missing
   ```
   root@romulus:~# coredumpctl list

   TIME                       PID UID GID SIG     COREFILE EXE                 SIZE

   Sat 2026-04-04 17:26:45... 415   0   0 SIGSEGV missing  /usr/bin/debugcrash    -
   ```

### Root Cause Analysis
1. Image size issue:
   - Debug symbols and tools significantly increase image size
   - OpenBMC flash size is limited (~32MB)
   > To debug the image size issue, I used du -sh * | sort -hr to rank the directories. It became clear that debug-tools was bloated and taking up most of the space.
    
2. Core dump issue:
   - systemd-coredump drops large files
   - /var storage limitation
  
### Solutions
1. Optimize image size:
   - Remove `tools-debug`
   - Avoid embedding debug symbols in target
   - Use split debug symbols:
     `IMAGE_GEN_DEBUGFS = "1"`
    
2. Enable core dump(QEMU):
   - Modify /etc/systemd/coredump.conf
     ```
     Storage=external
     ProcessSizeMax=0
     ExternalSizeMax=0
     ```

   - Enable core dump in shell
     ```
     ulimit -c unlimited
     ```

   - Redirect core pattern:
     ```
     echo "/tmp/core.%e.%p" > /proc/sys/kernel/core_pattern
     ```
     > core_pattern is the remote control for kernel behavior; it dictates where the Linux kernel directs memory data following a program crash

### Debugging with GDB
- scp the core file from QEMU to HOST VM
  ```
  scp -P 2222 root@localhost:/tmp/core.debugcrash.* ./debugcrash.core
  ```

- Debugging with Cross GDB
  ```
  arm-openbmc-linux-gnueabi-gdb debugcrash debugcrash.core
  ```

- In GDB
  ```
  (gdb)  set sysroot /home/henry/openbmc_sdk/sysroots/arm1176jzs-openbmc-linux-gnueabi
  (gdb) bt

  #0  0x0047680c in main () at /usr/src/debug/debugcrash/git/debugcrash.cpp:11
  ```
  > Set up the Sysroot so GDB knows where to locate the ARM libraries
