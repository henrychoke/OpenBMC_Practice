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

-  Monitored logs to confirm service restart triggers sensor reinitialization using another terminal
```
root@romulus:~# journalctl -f -u 'xyz.openbmc_project.Hwmon@-iio\x2dhwmon\x2dbattery.service'
Mar 01 03:51:32 romulus systemd[1]: Started Phosphor Hwmon Poller.
Mar 01 06:26:30 romulus systemd[1]: Stopping Phosphor Hwmon Poller...
Mar 01 06:26:30 romulus systemd[1]: xyz.openbmc_project.Hwmon@-iio\x2dhwmon\x2dbattery.service: Deactivated successfully.
Mar 01 06:26:30 romulus systemd[1]: Stopped Phosphor Hwmon Poller.
Mar 01 06:26:30 romulus systemd[1]: Started Phosphor Hwmon Poller.
Mar 01 06:26:56 romulus systemd[1]: Stopping Phosphor Hwmon Poller...
Mar 01 06:26:56 romulus systemd[1]: xyz.openbmc_project.Hwmon@-iio\x2dhwmon\x2dbattery.service: Deactivated successfully.
Mar 01 06:26:56 romulus systemd[1]: Stopped Phosphor Hwmon Poller.
Mar 01 06:26:56 romulus systemd[1]: Started Phosphor Hwmon Poller.
```
### 1.2 D-bus sensor inspection
> The target of this exercise is to verify that the D-Bus value reflects the processed sensor data.
- Located sensor service via busctl
```
root@romulus:~# busctl list | grep Hwmon 
:1.348                                                                                            679 phosphor-hwmon- root             :1.348        xyz.openbmc_project.Hwmon@-iio\x2dhwmon\x2dbattery.service                        -       -
:1.42                                                                                             294 phosphor-hwmon- root             :1.42         xyz.openbmc_project.Hwmon@-ahb-apb-pwm\x2dtacho\x2dcontroller\x401e786000.service -       -
xyz.openbmc_project.Hwmon-90dc5dd3857daeb224c11f832395c5c454995ef20ee9cda4c1747f544f1f8541.Hwmon1 679 phosphor-hwmon- root             :1.348        xyz.openbmc_project.Hwmon@-iio\x2dhwmon\x2dbattery.service                        -       -
xyz.openbmc_project.Hwmon-c802d5b0aa994ff2acc1a47875439b0307ccaf4671c4a383ba55073cebadade2.Hwmon1 294 phosphor-hwmon- root             :1.42         xyz.openbmc_project.Hwmon@-ahb-apb-pwm\x2dtacho\x2dcontroller\x401e786000.service -       -

root@romulus:~# busctl tree xyz.openbmc_project.Hwmon-90dc5dd3857daeb224c11f832395c5c454995ef20ee9cda4c1747f544f1f8541.Hwmon1
```
> The names of services on D-Bus will include the names we see in "systemctl + hardware path". Let's list all currently "alive" services using D-Bus, and then use grep to filter them

- Identified object path
```
root@romulus:~# busctl tree xyz.openbmc_project.Hwmon-90dc5dd3857daeb224c11f832395c5c454995ef20ee9cda4c1747f544f1f8541.Hwmon1
`- /xyz
  `- /xyz/openbmc_project
    `- /xyz/openbmc_project/sensors
      `- /xyz/openbmc_project/sensors/voltage
        `- /xyz/openbmc_project/sensors/voltage/vbat
```

- Read voltage value
1. get-property method
```
root@romulus:~# busctl get-property xyz.openbmc_project.Hwmon-90dc5dd3857daeb224c11f832395c5c454995ef20ee9cda4c1747f544f1f8541.Hwmon1 /xyz/openbmc_project/sensors/voltage/vbat xyz.openbmc_project.Sensor.Value Value
d 1.93725

```
2. Object Mapper
> Because in OpenBMC, the service name (with a hash value) of Hwmon is dynamically generated. We cannot hardcode the service name; we must first use the ObjectMapper to query who owns the object `/xyz/openbmc_project/sensors/voltage/vbat`, and then dynamically call `get-property`.
```
root@romulus:~# busctl call xyz.openbmc_project.ObjectMapper /xyz/openbmc_project/object_mapper xyz.openbmc_project.ObjectMapper GetObject sas "/xyz/openbmc_project/sensors/voltage/vbat" 0

a{sas} 1 "xyz.openbmc_project.Hwmon-90dc5dd3857daeb224c11f832395c5c454995ef20ee9cda4c1747f544f1f8541.Hwmon1" 5 "org.freedesktop.DBus.Introspectable" "org.freedesktop.DBus.Peer" "org.freedesktop.DBus.Properties" "xyz.openbmc_project.Sensor.Value" "xyz.openbmc_project.State.Decorator.OperationalStatus"

```
