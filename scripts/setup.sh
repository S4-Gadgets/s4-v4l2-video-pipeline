#!/bin/bash
set -e

echo "[S4] Installing S4-Gadgets Adaptive Video Pipeline..."
cd /

# Copy device tree overlay
sudo cp /s4-v4l2-video-pipeline/overlay/s4-video-pipeline-overlay.dts /boot/overlays/
sudo sed -i '/dtoverlay=tc358743/d' /boot/config.txt # Remove common HDMI video device to free up device video0
sudo sed -i '/dtoverlay=s4-video-pipeline-overlay/d' /boot/config.txt # Remove S4-Gadgets video pipeline encase this is a reinstall
echo 'dtoverlay=s4-video-pipeline-overlay' | sudo tee -a /boot/config.txt # Add S4-Gadgets video pipeline to create video0

# Install DKMS modules
mkdir /usr/src/s4videopipeline-1.0/
cp /s4-v4l2-video-pipeline/dkms/*.* /usr/src/s4videopipeline-1.0/
cp /s4-v4l2-video-pipeline/Makefile /usr/src/s4videopipeline-1.0/Makefile
cp /s4-v4l2-video-pipeline/drivers/*.* /usr/src/s4videopipeline-1.0/
cp /s4-v4l2-video-pipeline/include/*.* /usr/src/s4videopipeline-1.0/

cd /usr/src/s4videopipeline-1.0
sudo dkms add .
sudo dkms build s4videopipeline/1.0
sudo dkms install s4videopipeline/1.0
cd /

# Install udev rules
sudo cp /s4-v4l2-video-pipeline/udev/*.* /etc/udev/rules.d/
sudo udevadm control --reload-rules

# Install debug toggle tool
sudo cp /s4-v4l2-video-pipeline/scripts/s4-status.sh /usr/local/bin/s4-status.sh
sudo cp /s4-v4l2-video-pipeline/scripts/s4-video-debug.sh /usr/local/bin/s4-video-debug.sh
sudo chmod +x /usr/local/bin/s4-status.sh
sudo chmod +x /usr/local/bin/s4-video-debug.sh

# Install and Enable Services for Monitoring and Debug
sudo cp /s4-v4l2-video-pipeline/monitor/*.* /usr/local/bin/
sudo cp /s4-v4l2-video-pipeline/systemd/*.* /etc/systemd/system/
sudo systemctl daemon-reexec
sudo systemctl enable s4-video-monitor
sudo systemctl enable s4-debug-toggle.timer
sudo systemctl start s4-video-monitor

echo "[S4] Installation complete. Reboot recommended."
