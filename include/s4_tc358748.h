#ifndef S4_TC358748_H
#define S4_TC358748_H

#ifndef __S4_TC358748_H__
#define __S4_TC358748_H__

#include <media/v4l2-subdev.h>

struct tc358748_state {
    struct v4l2_subdev sd;
    struct v4l2_ctrl_handler ctrl_handler;
    struct media_pad pads[2];
    struct i2c_client *client;
    struct regmap *regmap;

    int hsync_last;
    int vsync_last;
    u32 width;
    u32 height;
    u32 framerate;
    u32 clock_rate;

    bool csi_active;
    bool bridge_enabled;
    bool clock_locked;
    bool passthrough_ready;
};

#endif // __S4_TC358748_H__
#endif /* S4_TC358748_H */