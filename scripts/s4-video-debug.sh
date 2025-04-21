#!/bin/bash
# S4 Debug Aggregator Tool
# Enables all debug features, collects diagnostics, then disables them after 1 hour

DEBUG_DIR="/sys/kernel/debug/s4-video"
LOG_FILE="/var/log/s4-video-debug.log"
CONTROL_TOOL="/usr/bin/v4l2-ctl"

echo "[S4 Debug] Enabling all diagnostics..."

# Enable V4L2 debug controls
$CONTROL_TOOL -d /dev/video0 -c enable_debug=1
$CONTROL_TOOL -d /dev/video0 -c enable_telemetry=1
$CONTROL_TOOL -d /dev/video0 -c enable_i2c_dump=1

# Optional: pull values from sysfs and dump them to log
mkdir -p $(dirname "$LOG_FILE")
{
    echo "=== S4 Video Diagnostics Dump: $(date) ==="
    echo "--- SYSFS ---"
    for file in "$DEBUG_DIR"/*; do
        echo "[$file]"
        cat "$file" 2>/dev/null || echo "(unreadable)"
        echo ""
    done
    echo "--- V4L2 Controls ---"
    $CONTROL_TOOL -d /dev/video0 --all
} >> "$LOG_FILE"

echo "[S4 Debug] Diagnostics logged to $LOG_FILE"
echo "[S4 Debug] Will auto-disable all debug features in 1 hour..."

# Background timer to disable debug
(sleep 3600
 echo "[S4 Debug] Auto-disabling diagnostics after timeout."
 $CONTROL_TOOL -d /dev/video0 -c enable_debug=0
 $CONTROL_TOOL -d /dev/video0 -c enable_telemetry=0
 $CONTROL_TOOL -d /dev/video0 -c enable_i2c_dump=0
 echo "[S4 Debug] All debug features disabled."
) &
