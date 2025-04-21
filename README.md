# S4-Gadgets Adaptive Video Pipeline

> Adaptive V4L2 video capture pipeline for Raspberry Pi using AD9984A and TC358748.  
> Includes automatic signal detection, telemetry, CSI bridge, and debug tools.

---

## 🚀 Quickstart

```bash
git clone https://github.com/S4-Gadgets/s4gadgets-vga-csi2-driver.git
chmod +x s4gadgets-vga-csi2-driver/scripts/setup.sh
sudo ./s4gadgets-vga-csi2-driver/scripts/setup.sh
sudo reboot
```

After reboot:

```bash
s4-status.sh
```

---

## 📦 Features

- Adaptive timing detection (sync, framerate, porches)
- V4L2 extended controls and telemetry
- DebugFS for real-time runtime status
- CSI-2 passthrough bridge with stability detection
- DKMS build system for automatic kernel module install
- Overlay-driven configuration with auto-linking endpoints
- Background resolution monitor and debug toggling
- Full systemd and udev integration

---

## 📂 Project Structure

```
.
├── drivers/                 # Kernel drivers for AD9984A and TC358748
├── include/                 # Header files for drivers
├── overlay/                 # Device tree overlay
├── scripts/                 # Setup, uninstall, debug control
├── monitor/                 # Runtime resolution change monitor
├── systemd/                 # Services for debug toggle and monitor
├── udev/                    # Udev rules for video device labeling
├── dkms/                    # DKMS config file
├── Makefile                 # Root makefile to build both modules
├── LICENSE                  # GPL v2 License
├── INSTALL.md               # Install instructions
├── UNINSTALL.md             # Uninstall instructions
└── README.md                # This file
```

---

## 🔧 Debug Mode

Temporarily enable diagnostics and telemetry with:

```bash
sudo s4-video-debug.sh
```

This enables all debug flags and will auto-disable after 1 hour.

---

## 🧪 View Telemetry

```bash
s4-status.sh
```

This summarizes:
- V4L2 controls
- DebugFS entries
- Service status

---

## 🧹 Uninstall

```bash
sudo ./s4gadgets-vga-csi2-driver/scripts/uninstall.sh
sudo reboot
```

This removes all services, drivers, and debug tools.

---

## 📜 License

Licensed under the [GPL v2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).