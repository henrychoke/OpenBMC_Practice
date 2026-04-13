#!/bin/bash

BMC_USER="root"
BMC_PASS="0penBmc"
BMC_HOST="localhost"

echo "connecting to $BMC_HOST and try get Redfish Token..."

RESPONSE=$(curl -k -s -i -H "Content-Type: application/json" \
  -X POST "https://$BMC_HOST/redfish/v1/SessionService/Sessions" \
  -d "{\"UserName\": \"$BMC_USER\", \"Password\": \"$BMC_PASS\"}")

TOKEN=$(echo "$RESPONSE" | grep -i "X-Auth-Token" | awk '{print $2}' | tr -d '\r

if [ -z "$TOKEN" ]; then
    echo "Error , cant get token"
    echo "$RESPONSE" | grep "{"
    exit 1
else
    echo "Get Token Successful！"
    echo "----------------------------------------"
    echo "$TOKEN"
    echo "----------------------------------------"
    export TOKEN=$TOKEN
fi
