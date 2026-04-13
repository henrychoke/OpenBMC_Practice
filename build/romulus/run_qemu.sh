./qemu-system-arm -m 256 -M romulus-bmc -nographic \
-drive file=tmp/deploy/images/romulus/obmc-phosphor-image-romulus.static.mtd,format=raw,if=mtd \
-net nic -net user,hostfwd=tcp:127.0.0.1:2222-:22,hostfwd=tcp:127.0.0.1:2443-:443,hostname=qemu \
-device at24c-eeprom,bus=aspeed.i2c.bus.3,address=0x50,rom-size=256
