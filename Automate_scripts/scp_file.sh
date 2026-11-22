#!/bin/bash

# Check if argument is provided
if [ $# -ne 1 ]; then
    echo "Usage: $0 /path/to/local/file.bin"
    exit 1
fi

LOCAL_FILE="$1"  # First argument
REMOTE_USER="petalinux"
REMOTE_IP="192.168.3.200"
REMOTE_PATH="/home/petalinux/"

# Remove old host key (avoid authenticity prompt)
ssh-keygen -f "$HOME/.ssh/known_hosts" -R "$REMOTE_IP"

# Copy file to remote
scp "$LOCAL_FILE" "$REMOTE_USER@$REMOTE_IP:$REMOTE_PATH"

# Optional: Run remote commands after upload
ssh "$REMOTE_USER@$REMOTE_IP" <<'EOF'
echo "File uploaded successfully."
# Add your FPGA commands here
# e.g.,
# sudo fpgautil -b /lib/firmware/Bitstream.bin
EOF

