
#ifndef __S4_TC358748_H__
#define __S4_TC358748_H__

#include <media/v4l2-subdev.h>

struct s4_tc358748_state {
    struct v4l2_subdev sd;
    struct v4l2_mbus_framefmt format;
    bool csi_ready;
};

#endif // __S4_TC358748_H__
