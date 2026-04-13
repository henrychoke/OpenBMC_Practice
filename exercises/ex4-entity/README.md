# Exercise4 Entity Manager

### Goal
Integrate a virtual hardware device into OpenBMC by leveraging Entity Manager and FRU Device.
This exercise demonstrates how OpenBMC dynamically discovers hardware components via FRU data and maps them into the D-Bus inventory system.

### Implementation
- Define a virtual device using Entity Manager:
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
