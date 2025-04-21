#!/bin/bash
set -e

echo "[S4] Installing S4-Gadgets Adaptive Video Pipeline..."
cd s4gadgets-vga-csi2-driver

# Copy device tree overlay
sudo cp ./overlay/s4-video-pipeline-overlay.dts /boot/overlays/
sudo sed -i '/dtoverlay=s4-video-pipeline-overlay/d' /boot/config.txt
echo 'dtoverlay=s4-video-pipeline-overlay' | sudo tee -a /boot/config.txt

# Install DKMS modules
mkdir /usr/src/s4videopipeline-1.0/
cp ./dkms/dkms.conf /usr/src/s4videopipeline-1.0/dkms.conf
cp ./Makefile /usr/src/s4videopipeline-1.0/Makefile

mkdir /usr/src/s4videopipeline-1.0/drivers/
cp ./drivers/*.* /usr/src/s4videopipeline-1.0/drivers/

mkdir /usr/src/s4videopipeline-1.0/include/
cp ./include/*.* /usr/src/s4videopipeline-1.0/include/

sudo dkms add s4videopipeline/1.0
sudo dkms build s4videopipeline/1.0
sudo dkms install s4videopipeline/1.0

# Install udev rules
sudo cp ./udev/99-s4-v4l2.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules

# Install monitor daemon
sudo cp ./monitor/s4-video-monitor.py /usr/local/bin/
sudo cp ./systemd/s4-video-monitor.service /etc/systemd/system/
sudo systemctl daemon-reexec
sudo systemctl enable s4-video-monitor
sudo systemctl start s4-video-monitor


# Install debug toggle tool
sudo cp ./scripts/s4-video-debug.sh /usr/local/bin/
sudo chmod +x /usr/local/bin/s4-video-debug.sh

# Install and enable debug toggle service and timer
sudo cp ./systemd/s4-debug-toggle.service /etc/systemd/system/
sudo cp ./systemd/s4-debug-toggle.timer /etc/systemd/system/
sudo systemctl daemon-reexec
sudo systemctl enable s4-debug-toggle.timer

echo "[S4] Installation complete. Reboot recommended."
