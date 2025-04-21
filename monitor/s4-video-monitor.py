#!/usr/bin/env python3

import subprocess
import time
import os

DEVICE = "/dev/s4-video"
POLL_INTERVAL = 2  # seconds

def get_control(name):
    try:
        out = subprocess.check_output(["v4l2-ctl", "-d", DEVICE, f"--get-ctrl={name}"], text=True)
        return int(out.strip().split(":")[-1].strip())
    except Exception:
        return None

def main():
    print("[S4] Starting video signal monitor...")

    prev_res = (None, None)
    prev_fps = None

    while True:
        hres = get_control("s4_horizontal_resolution")
        vres = get_control("s4_vertical_resolution")
        fps  = get_control("s4_estimated_framerate")

        if None not in (hres, vres, fps):
            if (hres, vres) != prev_res or fps != prev_fps:
                print(f"[S4] Video signal changed: {hres}x{vres} @ {fps}fps")
                prev_res = (hres, vres)
                prev_fps = fps

        time.sleep(POLL_INTERVAL)

if __name__ == "__main__":
    main()
