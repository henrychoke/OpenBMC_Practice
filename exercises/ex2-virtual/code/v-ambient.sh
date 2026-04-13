!/bin/sh
# 1. defining a persistant service
SERVICE="xyz.openbmc_project.VAmbient"
OBJ_PATH="/xyz/openbmc_project/sensors/temperature/v_ambient"
INTERFACE="xyz.openbmc_project.Sensor.Value"
REG_FILE="/home/root/v_ambient_raw"

# Initialize a value when the file is not exist
if [ ! -f "$REG_FILE" ]; then
    echo "25000" > "$REG_FILE"
fi

# 2. Register this in Dbus

# Before that make sure service is online sucessfull
busctl --system call org.freedesktop.DBus /org/freedesktop/DBus org.freedesktop.DBus RequestName su "$SERVICE" 4

# Force ObjectMapper update cache(discover the new service)
busctl --system call xyz.openbmc_project.ObjectMapper /xyz/openbmc_project/object_mapper xyz.openbmc_project.ObjectMapper Tri

echo "Dbus service in online, start monitoring value..."

# Start cycle update
while true; do
    RAW=$(cat "$REG_FILE")
    VALUE=$(awk "BEGIN {print $RAW/1000}")

    # Add busctl implementation over here
    busctl --system set-property "$SERVICE" "$OBJ_PATH" "$INTERFACE" Value d "$VALUE"

    echo "Current temp: $VALUE degree (Original: $RAW)"
    sleep 5
done
