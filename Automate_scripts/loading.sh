#!/bin/bash
set -euo pipefail

if [ "$#" -ne 4 ]; then
    echo "Usage: sudo ./demo_script.sh <wrapper_bin> <dtbo_file> <bitstream1> <bitstream2>"
    exit 1
fi

WRAPPER_BIN="$1"
DTBO_FILE="$2"
BIT1="$3"
BIT2="$4"
FW_DIR="/lib/firmware"

# Use basenames so the files in /lib/firmware keep the original names you provided
WRAPPER_BASENAME="$(basename "$WRAPPER_BIN")"
DTBO_BASENAME="$(basename "$DTBO_FILE")"
BIT1_BASENAME="$(basename "$BIT1")"
BIT2_BASENAME="$(basename "$BIT2")"

WRAPPER_DEST="$FW_DIR/$WRAPPER_BASENAME"

echo "=========================================================="
echo " DEMO: Dynamic PL Bitstream Upload and GPIO Read Example"
echo "=========================================================="

echo
echo "Step 0: sanity checks..."
if [ ! -f "$WRAPPER_BIN" ]; then
    echo "Error: wrapper file not found: $WRAPPER_BIN"
    exit 1
fi
if [ ! -f "$DTBO_FILE" ]; then
    echo "Error: dtbo file not found: $DTBO_FILE"
    exit 1
fi
if [ ! -f "$BIT1" ]; then
    echo "Error: bitstream1 not found: $BIT1"
    exit 1
fi
if [ ! -f "$BIT2" ]; then
    echo "Error: bitstream2 not found: $BIT2"
    exit 1
fi
echo "All input files found."

echo
echo "Step 1: Copying wrapper bitstream ($WRAPPER_BIN) to firmware folder as $WRAPPER_BASENAME..."
cp "$WRAPPER_BIN" "$WRAPPER_DEST"
echo "Done."

echo
echo "Step 2: Copying device tree overlay ($DTBO_FILE) to firmware folder as $DTBO_BASENAME..."
cp "$DTBO_FILE" "$FW_DIR/$DTBO_BASENAME"
echo "Done."

echo
echo "Step 3: Copying other bitstreams ($BIT1, $BIT2) to firmware folder..."
cp "$BIT1" "$FW_DIR/$BIT1_BASENAME"
cp "$BIT2" "$FW_DIR/$BIT2_BASENAME"
echo "Done."

echo
echo "Step 4: Loading wrapper and overlay into PL..."
fpgautil -b "$WRAPPER_DEST" -o "$FW_DIR/$DTBO_BASENAME"
echo "Wrapper and overlay loaded successfully!"

echo
echo "Step 5: Detecting PL GPIO pins..."
BASE_GPIO=$(grep -l "80000000.gpio" /sys/class/gpio/gpiochip*/label 2>/dev/null | grep -o '[0-9]\+' || true)
if [ -z "$BASE_GPIO" ]; then
    echo "Error: Could not find GPIO base for 80000000.gpio"
    exit 1
fi
echo "Detected base GPIO: $BASE_GPIO"

echo
echo "Step 6: Exporting 4 consecutive GPIOs starting from $BASE_GPIO..."
for ((i=BASE_GPIO; i<BASE_GPIO+4; i++)); do
    if [ ! -d "/sys/class/gpio/gpio$i" ]; then
        echo "$i" > /sys/class/gpio/export || true
        echo "GPIO $i exported."
    else
        echo "GPIO $i already exported."
    fi
done
echo "All GPIOs ready."

# helper to read 4 gpio values as a binary string
read_gpio_binary() {
    BIN_OUTPUT=""
    for ((i=BASE_GPIO; i<BASE_GPIO+4; i++)); do
        VALUE=$(cat /sys/class/gpio/gpio$i/value 2>/dev/null || echo "0")
        BIN_OUTPUT="${BIN_OUTPUT}${VALUE}"
    done
    echo "$BIN_OUTPUT"
}

echo
echo "Step 7: Loading Bitstream 1 ($BIT1_BASENAME) into PL..."
fpgautil -b "$FW_DIR/$BIT1_BASENAME"
VALUES1=$(read_gpio_binary)
echo "Bitstream 1 loaded. Current GPIO values (binary): $VALUES1"

echo
echo "Step 8: Loading Bitstream 2 ($BIT2_BASENAME) into PL..."
fpgautil -b "$FW_DIR/$BIT2_BASENAME"
VALUES2=$(read_gpio_binary)
echo "Bitstream 2 loaded. Current GPIO values (binary): $VALUES2"

echo
echo "=========================================================="
echo " DEMO COMPLETE: Bitstreams uploaded and GPIO values read!"
echo "=========================================================="
