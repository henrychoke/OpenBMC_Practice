# Exercise1 - Sensor Debugging & Data Flow Analysis

## Goal
Understand and debug the full sensor data flow in OpenBMC:
- systemd service behavior
- D-Bus sensor interface
- hwmon (kernel) data source

## Investigation Steps
### 1.1 Service Behavior(systemd)
- Listed services using systemctl\
```
root@romulus:~# systemctl list-units --type=service
```

- Check battery service status
```
root@romulus:~# systemctl status "xyz.openbmc_project.Hwmon*-iio*"2dh~# systemctl status "xyz.openbmc_project.Hwmon*-iio*"
* xyz.openbmc_project.Hwmon@-iio\x2dhwmon\x2dbattery.service - Phosphor Hwmon Poller
     Loaded: loaded (/usr/lib/systemd/system/xyz.openbmc_project.Hwmon@.service; static)ervice'
     Active: active (running) since Sun 2026-03-01 07:26:32 UTC; 27min ago\x2dbat
 Invocation: 9ceff84308e046ecb8f82cd2a3fdecf7
   Main PID: 679 (phosphor-hwmon-)
        CPU: 98ms
     CGroup: /system.slice/system-xyz.openbmc_project.Hwmon.slice/xyz.openbmc_project.Hwmon@-iio\x2dhwmon\x2dbattery.service
             `-679 /usr/bin/phosphor-hwmon-readd -i 90dc5dd3857daeb224c11f832...
```

- Observed hwmon service lifecycle
```
root@romulus:~# systemctl restart 'xyz.openbmc_project.Hwmon@-iio\x2dhwmon\x2dbattery.service'
```
