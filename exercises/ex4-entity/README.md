# Exercise4 Entity Manager

## Goal
Integrate a virtual hardware device into OpenBMC by leveraging Entity Manager and FRU Device.
This exercise demonstrates how OpenBMC dynamically discovers hardware components via FRU data and maps them into the D-Bus inventory system.

## Implementation
- **Define a virtual device using Entity Manager**
  1. Created a JSON configuration to describe a virtual board and sensor(QEMU):
     ```
     {
         "Exposes": [
          {
            "Name": "Henry_Virtual_Sensor",
            "Type": "ADC"
          }
        ],
        "Name": "Henry_Experimental_Board",
        "Probe": "TRUE",
        "Type": "Board"
     }
     ```
  2. Restart Entity-Manager service
     ```
     systemctl restart xyz.openbmc_project.EntityManager
     ```
  3. The device appears in D-Bus inventory
     ```
        root@romulus:~# busctl tree xyz.openbmc_project.EntityManager
      `- /xyz
        `- /xyz/openbmc_project
          |- /xyz/openbmc_project/EntityManager
          `- /xyz/openbmc_project/inventory
            `- /xyz/openbmc_project/inventory/system
              `- /xyz/openbmc_project/inventory/system/board
                `- /xyz/openbmc_project/inventory/system/board/Henry_Experimental_Board
                  `- /xyz/openbmc_project/inventory/system/board/Henry_Experimental_Board/Henry_Virtual_Sensor
      ```

- **Emulate real hardware via FRU (EEPROM)**
  > Using static "Probe": "TRUE" is not realistic for real systems because:
  > 1. BMC must detect actual hardware presence
  > 2. Cannot rely on static configuration
  
  1. Since QEMU lacks physical FRU, I attached a virtual EEPROM via QEMU ( [run_qemu.sh](../../build/romulus/run_qemu.sh) ):
     ```
     -device at24c-eeprom,bus=aspeed.i2c.bus.3,address=0x50,rom-size=256
     ```
  2. Dynamically registered I2C device
     ```
     root@romulus:~# echo 24c02 0x50 > /sys/bus/i2c/devices/i2c-3/new_device
     [  103.331977] at24 3-0050: 256 byte 24c02 EEPROM, writable, 1 bytes/write
     [  103.332949] i2c i2c-3: new_device: Instantiated device 24c02 at 0x50

     root@romulus:~# ls -l /sys/bus/i2c/devices/i2c-3/3-0050/eeprom
     -rw-------    1 root     root           256 Apr  9 08:21 /sys/bus/i2c/devices/i2c-3/3-0050/eeprom
     ```
     > Register a device of type 24c02 at address 0x50 on i2c-3 and verified it

  3. Generate valid FRU data (pls refer to the [IPMI FRU doc](https://www.intel.com/content/dam/www/public/us/en/documents/specification-updates/ipmi-platform-mgt-fru-info-storage-def-v1-0-rev-1-3-spec-update.pdf)):
       - Implemented a Python script [FRU_create.py](code/FRU_create.py) to construct a valid FRU binary, which included FRU Header + Board Info Area + Checksum calculation, and `scp` into the QEMU
       - The key constraint must be satisfied by the following calculation:
         > (Sum of all bytes + checksum) mod 256 = 0. 
         > The last byte must be a checksum such that the sum of all data bytes plus the checksum equals zero   modulo 256. For example, if the total sum of the 8-byte Header and Board Info Area is 0x2CE, the checksum should be 0x32, because $0x2CE + 0x32 = 0x300$, which is 0 mod 256
       - This ensures the FRU data is valid and readable by the system

  4. Inject FRU into EEPROM
     ```
     root@romulus:~#  cat /tmp/henry_fru.bin > /sys/bus/i2c/devices/i2c-3/3-0050/eeprom
     ```

  5. Trigger hardware discovery and check whether the device is dynamically detected
     ```
     root@romulus:~# systemctl restart xyz.openbmc_project.FruDevice
     root@romulus:~# busctl tree xyz.openbmc_project.FruDevice
     `- /xyz
       `- /xyz/openbmc_project
         `- /xyz/openbmc_project/FruDevice
           `- /xyz/openbmc_project/FruDevice/Henry_Virtual_Card
     ```
## Key Findings
- How OpenBMC discovers hardware dynamically (not statically)
- Relationship between:
   - EEPROM (FRU)
   - FruDevice service
   - Entity Manager
   - D-Bus inventory
- Realistic platform bring-up flow without physical hardware
