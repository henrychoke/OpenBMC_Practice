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

### 1.3 Kernel Source inspection
> In this case, when monitoring the battery voltage of an object, the BMC cannot directly input the object's voltage into the BMC board. Therefore, there will be a voltage drop. However, the value seen on the web interface must match. This requires multiplying the gain value in the .conf file before letting D-Bus read it correctly.
- Find the vbat PID
```
root@romulus:~# ps ww | grep phosphor-hwmon

  294 root     10648 S    /usr/bin/phosphor-hwmon-readd -i c802d5b0aa994ff2acc1a47875439b0307ccaf4671c4a383ba55073cebadade2 -o /ahb/apb/pwm-tacho-controller@1e786000

  679 root     10648 S    /usr/bin/phosphor-hwmon-readd -i 90dc5dd3857daeb224c11f832395c5c454995ef20ee9cda4c1747f544f1f8541 -o /iio-hwmon-battery

 1246 root      2988 S    grep phosphor-hwmon
```
- Locate the file that defines the voltage; the path after -o will be mapped to the /etc/default/obmc/hwmon/ folder
```
root@romulus:~# find /etc/default/obmc/hwmon/ -name "*.conf"
/etc/default/obmc/hwmon/ahb/apb/bus@1e78a000/i2c@440/w83773g@4c.conf
/etc/default/obmc/hwmon/ahb/apb/pwm-tacho-controller@1e786000.conf
/etc/default/obmc/hwmon/devices/platform/gpio-fsi/fsi0/slave@00--00/00--00--00--06/sbefifo1-dev0/occ-hwmon.1.conf
/etc/default/obmc/hwmon/devices/platform/gpio-fsi/fsi0/slave@00--00/00--00--00--0a/fsi1/slave@01--00/01--01--00--06/sbefifo2-dev0/occ-hwmon.2.conf
/etc/default/obmc/hwmon/iio-hwmon-battery.conf

root@romulus:~# cat /etc/default/obmc/hwmon/iio-hwmon-battery.conf

LABEL_in1=vbat

GAIN_in1=2.25

```

> The -o parameter (e.g., -o /iio-hwmon-battery) corresponds to the Device Tree path in the Linux Kernel
