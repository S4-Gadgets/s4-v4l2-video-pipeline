
#ifndef __S4_AD9984A_H__
#define __S4_AD9984A_H__

#include <media/v4l2-subdev.h>

struct s4_ad9984a_state {
    struct v4l2_subdev sd;
    struct v4l2_ctrl_handler ctrl_handler;
    struct v4l2_mbus_framefmt format;
    u32 h_active;
    u32 v_active;
    u32 frame_rate;
    bool signal_present;
};

#endif // __S4_AD9984A_H__
