# Installation Instructions – S4-Gadgets Adaptive Video Pipeline

This project enables full analog VGA-to-CSI capture via V4L2, with real-time timing detection, telemetry, and debug control.

---

## ✅ Prerequisites

- Raspberry Pi (or compatible) with CSI-2 interface enabled
- I²C enabled (`raspi-config` or `/boot/config.txt`)
- Kernel headers installed (`sudo pacman -S linux-headers` or equivalent)
- `v4l-utils`, `DKMS`, and `make` available

---

## 🚀 Quickstart

```bash
git -C / clone https://github.com/S4-Gadgets/s4-v4l2-video-pipeline.git
chmod +x /s4-v4l2-video-pipeline/scripts/setup.sh
sudo /s4-v4l2-video-pipeline/scripts/setup.sh
sudo reboot
```

---

## 🧪 Verify System Status

After reboot, run:

```bash
s4-status.sh
```

This will show:
- Current V4L2 telemetry values
- DebugFS timing sync info
- Debug service status

---

## 🧰 Enable Debug Mode (Temporary)

```bash
sudo s4-video-debug.sh
```

This will enable all telemetry flags for 1 hour and auto-disable.

---

## 🧼 To Uninstall

```bash
sudo /s4-v4l2-video-pipeline/scripts/uninstall.sh
sudo reboot
```

This removes all drivers, systemd services, debug utilities, and DKMS traces.

---

Licensed under GPL v2.