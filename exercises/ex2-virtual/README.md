# Exercise2 Virtual Sensor
## Goal
Implement a virtual device on OpenBMC and expose its data via D-Bus using sdbusplus.

## Investigation Steps
- I initially tried writing a [script](code/v_ambient.sh) directly in QEMU to publish the service, but later found that I couldn't requestName, so it failed to register on D-Bus.

- Next, I tried using sdbusplus: I packaged the SDK on a Ubuntu VM, cross-compiled the C++ files using the Romulus environment, and then scp'd the binary file into QEMU
  1. For the SDK, please refer to the OpenBMC wiki
     ```
     source environment-setup-arm1176jzs-openbmc-linux-gnueabi
     ```
  2. Then, use CMake to compile the cpp file
     - [CMakeLists.txt](../../meta-henry/recipes-vambient/v-ambient/files/CMakeLists.txt)
     - [v_ambient.cpp](../../meta-henry/recipes-vambient/v-ambient/files/v-ambient.cpp)
     - Host VM
       ```
       mkdir build && cd build
       cmake ..
       make

       scp -P 2222 v-ambient root@localhost:/home/root/
       ```
     - QEMU
       ```
       root@romulus:~# chmod +x /home/root/v-ambient
       root@romulus:~# /home/root/v-ambient
       ```
  3. I later found that the WebUI couldn't detect this virtual sensor, so I used Redfish to check the full sensor list.
     - Romulus build lacks the curl command. To include it, add the following to [local.conf](../../build/romulus/conf/local.conf) and rebuild the image
       ```
       IMAGE_INSTALL:append = " curl"
       ```
     -  Obtain a session token from the BMC before the curl request, pls refer to the [check_token.sh](code/check_token.sh)
        ```
        root@romulus:~/scripts# chmod +x check_token.sh
        root@romulus:~/scripts# source ./check_token.sh
        connecting to localhost and try get Redfish Token...
        Get Token Successful！
        ----------------------------------------
        E6LqgBXXXXXXXXX
        ```
        > A token is a temporary security credential obtained via login to authorize Redfish API requests.
      - Curl results using $token show an empty 'Members' field; no sensors matching Redfish specifications were detected
        ```
        root@romulus:~/scripts# curl -k -H "X-Auth-Token: $TOKEN"
        https://localhost/redfish/v1/Chassis/chassis/Sensors
        {
          "@odata.id": "/redfish/v1/Chassis/chassis/Sensors",
          "@odata.type": "#SensorCollection.SensorCollection",
          "Description": "Collection of Sensors for this Chassis",
          "Members": [],
          "Members@odata.count": 0,
          "Name": "Sensors"<img width="904" height="138" alt="image" src="https://github.com/user-attachments/assets/a59b0942-10a3-4aa0-b152-20e83cab8537" />
        }
        ```
  4. I hypothesize that the missing chassis association for this D-Bus object is the cause; therefore, an association definition is required.
     - [v_ambient_assoc.cpp](../../meta-henry/recipes-vambient/v-ambient/files/v-ambient-assoc.cpp)
  > My understanding is that the D-Bus object's interfaces and properties must adhere to Redfish specifications so that bmcweb can discover and expose them to the WebUI
  
  5. Repeat the process in step 3, then run introspection to see the following results:
     <img width="1339" height="369" alt="introspec" src="https://github.com/user-attachments/assets/a5a17c44-7f86-4a5a-8451-bb90e40639ba" />
     
  6. Now, the WebUI is also displaying the items correctly:
     <img width="1568" height="378" alt="webui" src="https://github.com/user-attachments/assets/29acf814-6915-4655-a640-76d3e8eba23c" />

- Lastly, I added it to the systemd units and updated the Yocto recipe to include it in the image build, making it ready to use on startup. For further details, please refer to the [meta-henry](../../meta-henry) layer
  > In [v-ambient_git.bb](../../meta-henry/recipes-vambient/v-ambient/v-ambient_git.bb), due to the Yocto Whinlatter version (as specified in [layer.conf](../../meta-henry/conf/layer/conf)), source files can no longer be placed directly in `$WORKDIR` after compilation. Therefore, the subdir approach was implemented.
