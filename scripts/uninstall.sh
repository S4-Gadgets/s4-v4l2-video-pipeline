#!/bin/bash
set -e

echo "[S4] Uninstalling S4-Gadgets Adaptive Video Pipeline..."

# Remove overlay config from boot config
sudo sed -i '/dtoverlay=s4-video-pipeline-overlay/d' /boot/config.txt

# Delete overlay binary
sudo rm -f /boot/overlays/s4-video-pipeline-overlay.dts

# Remove udev rule
sudo rm -f /etc/udev/rules.d/99-s4-v4l2.rules
sudo udevadm control --reload-rules

# Uninstall DKMS modules
dkms remove s4videopipeline/1.0 --all || true

# Stop and remove monitor service
sudo systemctl stop s4-video-monitor || true
sudo systemctl disable s4-video-monitor || true
sudo rm -f /usr/local/bin/s4-video-monitor.py
sudo rm -f /etc/systemd/system/s4-video-monitor.service
sudo systemctl daemon-reexec

# Uninstall debug toggle tool and service
sudo systemctl stop s4-debug-toggle.timer || true
sudo systemctl disable s4-debug-toggle.timer || true
sudo rm -f /usr/local/bin/s4-video-debug.sh
sudo rm -f /etc/systemd/system/s4-debug-toggle.service
sudo rm -f /etc/systemd/system/s4-debug-toggle.timer
sudo systemctl daemon-reexec

# Remove S4-Gadgets videopipeline drivers
sudo rm -r /usr/src/s4videopipeline*

# Remove S4-Gadgets videopipeline git repository download
sudo rm -r /s4-v4l2-video-pipeline*

echo "[S4] Uninstall complete. Please reboot to finalize cleanup."







##### MANUAL PROCESS OF FORCIBLY CLEANING UP DKMS ENTIRES ####

## First try the graceful method
#dkms remove s4videopipeline/1.0 --all || true

## Remove the broken entry by force
# sudo rm -rf /var/lib/dkms/%MODULE_NAME%/1.0
#sudo rm -rf /var/lib/dkms/s4videopipeline/1.0 # Possible Example

## Check the status again, no more "broken" entries listed
#dkms status

## Clean up any leftover bad links (optional safety)
# sudo find /usr/src -name "*%MODULE_NAME%*" -exec rm -rf {} +
#sudo find /usr/src -name "*s4videopipeline*" -exec rm -rf {} + # Possible Example
