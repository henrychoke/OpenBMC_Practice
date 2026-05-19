# OpenBMC Practice

## Overview
This repo contains my hands-on exercises on OpenBMC development.

## Exercises
- Sensor Debugging & Data Flow Analysis
- Virtual Sensor
- Crash Service Debug
- Entity Manager

## Structure
See each exercise for details:
- [Exercise 1 - Sensor Debugging & Data Flow Analysis](exercises/ex1-sensor)
- [Exercise 2 - Virtual Sensor](exercises/ex2-virtual)
- [Exercise 3 - Crash Service Debug](exercises/ex3-crash)
- [Exercise 4 - Entity Manager](exercises/ex4-entity)
- [Exercise 5 - IIO sensor integration](exercises/ex5-max30102-iio-hwmon)
- [Exercise 6 - I2C Daemon](exercises/ex6-max30102-ioctl)

## Development Environment
###  Virtual Machine and Host OS
* **OS:** Ubuntu 22.04.5 LTS (x86_64)
* **Memory:** 16GB
* **Disk Space:** 250GB

###  Target Architecture
1. QEMU
* **Machine:** Romulus BMC
* **Architecture:** ARM1176JZS
* **SDK:** OpenBMC OESDK (arm1176jzs-openbmc-linux-gnueabi)

2. Raspberry Pi 4 (Physical Board)
* **Machine:** `raspberrypi4-64`
* **Architecture:** ARMv8-A (Cortex-A72, aarch64)
* **Kernel Version:** 6.12.x (Linux-RaspberryPi)
* **OpenBMC Image:** obmc-phosphor-image

