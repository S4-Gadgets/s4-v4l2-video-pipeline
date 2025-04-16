// SPDX-License-Identifier: GPL-2.0
// Copyright (C) 2025 Michael Scott, S4-Gadgets
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/v4l2-subdev.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-fwnode.h>

struct s4_ad9984a {
    struct v4l2_subdev sd;
    struct v4l2_ctrl_handler ctrl_handler;
    struct mutex lock;

    // Detected video parameters
    u32 detected_width;
    u32 detected_height;
    u32 detected_fps;
};

static int s4_ad9984a_query_dv_timings(struct v4l2_subdev *sd,
                                       struct v4l2_dv_timings *timings)
{
    struct s4_ad9984a *dev = container_of(sd, struct s4_ad9984a, sd);
    // Simulated dynamic timing detection
    timings->bt.width = dev->detected_width;
    timings->bt.height = dev->detected_height;
    timings->bt.pixelclock = dev->detected_fps * dev->detected_width * dev->detected_height;
    return 0;
}

static const struct v4l2_subdev_video_ops s4_video_ops = {
    .query_dv_timings = s4_ad9984a_query_dv_timings,
};

static const struct v4l2_subdev_ops s4_subdev_ops = {
    .video = &s4_video_ops,
};

static int s4_probe(struct i2c_client *client,
                    const struct i2c_device_id *id)
{
    struct s4_ad9984a *dev;
    struct v4l2_subdev *sd;

    dev = devm_kzalloc(&client->dev, sizeof(*dev), GFP_KERNEL);
    if (!dev)
        return -ENOMEM;

    mutex_init(&dev->lock);
    sd = &dev->sd;

    v4l2_i2c_subdev_init(sd, client, &s4_subdev_ops);
    sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;

    dev_info(&client->dev, "S4 AD9984A adaptive decoder loaded");

    return 0;
}

static const struct i2c_device_id s4_ad9984a_id[] = {
    { "ad9984a", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, s4_ad9984a_id);

static struct i2c_driver s4_ad9984a_driver = {
    .driver = {
        .name = "s4_ad9984a",
    },
    .probe = s4_probe,
    .id_table = s4_ad9984a_id,
};
module_i2c_driver(s4_ad9984a_driver);

MODULE_AUTHOR("Michael Scott <support@s4gadgets.com>");
MODULE_DESCRIPTION("S4-Gadgets AD9984A Adaptive V4L2 Driver");
MODULE_LICENSE("GPL");
