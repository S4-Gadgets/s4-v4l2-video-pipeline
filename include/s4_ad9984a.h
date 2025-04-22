#ifndef S4_AD9984A_H
#define S4_AD9984A_H

#ifndef __S4_AD9984A_H__
#define __S4_AD9984A_H__

#include <media/v4l2-subdev.h>

struct s4_ad9984a_state {
    struct v4l2_subdev sd;
    struct v4l2_ctrl_handler ctrl_handler;
    struct v4l2_mbus_framefmt format;
    struct media_pad pad;
    struct i2c_client *client;

    // Telemetry state
    u32 h_active;
    u32 v_active;
    u32 framerate;

    u32 hsync_len;
    u32 vsync_len;
    u32 hbp;
    u32 vbp;
    u32 hfp;
    u32 vfp;
    u32 fps;
    u32 pixelclock;
    bool signal_present;
};

#endif // __S4_AD9984A_H__
#endif /* S4_AD9984A_H */