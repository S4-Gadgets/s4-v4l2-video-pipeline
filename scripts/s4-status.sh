#!/bin/bash
# s4-status.sh - Show status of S4-Gadgets video pipeline (AD9984A + TC358748)

GREEN="\033[0;32m"
YELLOW="\033[1;33m"
RED="\033[0;31m"
NC="\033[0m" # No Color

print_header() {
    echo -e "${YELLOW}==== S4-GADGETS VIDEO PIPELINE STATUS ====${NC}"
}

check_module() {
    local mod=$1
    if lsmod | grep -q "$mod"; then
        echo -e "${GREEN}✓${NC} Kernel module '$mod' is loaded"
    else
        echo -e "${RED}✗${NC} Kernel module '$mod' is NOT loaded"
    fi
}

check_devnode() {
    local node=$1
    if [ -e "$node" ]; then
        echo -e "${GREEN}✓${NC} Found device node: $node"
    else
        echo -e "${RED}✗${NC} Missing device node: $node"
    fi
}

check_debugfs() {
    local path=$1
    if [ -d "$path" ]; then
        echo -e "${GREEN}✓${NC} DebugFS path exists: $path"
        ls "$path"
    else
        echo -e "${RED}✗${NC} DebugFS path missing: $path"
    fi
}

print_header

echo "\n[MODULES]"
check_module s4_ad9984a
check_module s4_tc358748

echo "\n[DEVICE NODES]"
check_devnode /dev/v4l-subdev0
check_devnode /dev/v4l-subdev1

echo "\n[DEBUGFS]"
check_debugfs /sys/kernel/debug/s4_ad9984a
check_debugfs /sys/kernel/debug/s4_tc358748

exit 0
