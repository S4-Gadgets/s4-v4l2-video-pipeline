# Uninstall Instructions – S4-Gadgets V4L2 Video Pipeline

This will remove all drivers, debug tools, and systemd integration from your system.

---

## 🔧 Run the Uninstall Script

From your cloned repo directory:

```bash
chmod +x scripts/uninstall.sh
sudo ./scripts/uninstall.sh
```

---

## 🧹 What This Removes

- Kernel modules (via DKMS)
- `s4-status.sh`, `s4-video-debug.sh`, etc.
- `s4-debug-toggle.service` and `.timer`
- `s4-video-monitor.service`
- Udev rules (`99-s4-v4l2.rules`)
- Device tree overlay links

---

## 🌀 Optional: Clean Overlay Config

Manually remove this line from `/boot/config.txt` if present:

```
dtoverlay=s4-video-pipeline-overlay
```

Then reboot to finalize removal:

```bash
sudo reboot
```

---

## 💡 DKMS Cleanup (Manual Option)

If needed:

```bash
sudo dkms remove -m s4videopipeline -v 1.0 --all
```

---

## ✅ Confirm Cleanup

After reboot:

```bash
lsmod | grep s4
systemctl list-units --type=service | grep s4
```

If nothing appears, the stack has been fully removed.

---

Licensed under GPL v2.