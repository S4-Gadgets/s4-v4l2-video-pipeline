// SPDX-License-Identifier: GPL-2.0
/*
 * S4 AD9984A Analog VGA to RGB888 V4L2 Driver with Adaptive Timing
 * Author: Michael Scott <support@s4gadgets.com>
 * https://www.s4gadgets.com/
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/regmap.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-ctrls.h>
#include <linux/debugfs.h>
#include "s4_ad9984a.h"

// Define unique control IDs
#define V4L2_CID_S4_ENABLE_DEBUG (V4L2_CID_USER_BASE + 0x1000)
#define V4L2_CID_S4_HSYNC_LEN     (V4L2_CID_USER_BASE + 0x1100)
#define V4L2_CID_S4_VSYNC_LEN     (V4L2_CID_USER_BASE + 0x1101)
#define V4L2_CID_S4_HBP           (V4L2_CID_USER_BASE + 0x1102)
#define V4L2_CID_S4_VBP           (V4L2_CID_USER_BASE + 0x1103)
#define V4L2_CID_S4_HFP           (V4L2_CID_USER_BASE + 0x1104)
#define V4L2_CID_S4_VFP           (V4L2_CID_USER_BASE + 0x1105)
#define V4L2_CID_S4_FRAMERATE     (V4L2_CID_USER_BASE + 0x1106)
#define V4L2_CID_S4_PIXELCLOCK    (V4L2_CID_USER_BASE + 0x1107)

static int debug_enabled = 0;

// Control operations
static int s4_s_ctrl(struct v4l2_ctrl *ctrl)
{
    switch (ctrl->id) {
    case V4L2_CID_S4_ENABLE_DEBUG:
        debug_enabled = ctrl->val;
        pr_info("S4 debug %s\n", debug_enabled ? "ENABLED" : "DISABLED");
        break;
    default:
        return -EINVAL;
    }
    return 0;
}

static const struct v4l2_ctrl_ops s4_ctrl_ops = {
    .s_ctrl = s4_s_ctrl,
};

// Video sub-device state
#define AD9984A_NAME "s4_ad9984a"

struct s4_ad9984a_state {
    struct v4l2_subdev sd;
    struct v4l2_ctrl_handler ctrl_handler;
    struct media_pad pad;

    struct i2c_client *client;
    struct regmap *regmap;

    // Adaptive telemetry
    u32 h_active;
    u32 v_active;
    u32 framerate;
    u64 last_sync;
};					  

// Read video timing info
static int s4_ad9984a_read_timing(struct s4_ad9984a_state *state)
{
    // Placeholder: replace with real sync pulse edge detection or sync timer
    state->h_active = 640;
    state->v_active = 480;
    state->framerate = 60;
    state->hsync_len = 96;
    state->vsync_len = 2;
    state->hbp = 48;
    state->vbp = 33;
    state->hfp = 16;
    state->vfp = 10;
    state->fps = 60;
    state->pixelclock = 25175000;
    return 0;
}

// Sub-device operations
static int s4_ad9984a_s_stream(struct v4l2_subdev *sd, int enable)
{
    struct s4_ad9984a_state *state = container_of(sd, struct s4_ad9984a_state, sd);

    if (enable) {
        s4_ad9984a_read_timing(state);
        dev_info(&state->client->dev, "Detected %ux%u @ %uHz\n",
                 state->h_active, state->v_active, state->framerate);
    }

    return 0;
}

static int s4_ad9984a_enum_mbus_code(struct v4l2_subdev *sd,
                                     struct v4l2_subdev_state *cfg,
                                     struct v4l2_subdev_mbus_code_enum *code)
{
    if (code->index > 0)
        return -EINVAL;

    code->code = MEDIA_BUS_FMT_RGB888_1X24;
    return 0;
}

static int s4_ad9984a_get_fmt(struct v4l2_subdev *sd,
                              struct v4l2_subdev_state *cfg,
                              struct v4l2_subdev_format *fmt)
{
    struct s4_ad9984a_state *state = container_of(sd, struct s4_ad9984a_state, sd);

    fmt->format.width = state->h_active;
    fmt->format.height = state->v_active;
    fmt->format.code = MEDIA_BUS_FMT_RGB888_1X24;
    fmt->format.field = V4L2_FIELD_NONE;
    fmt->format.colorspace = V4L2_COLORSPACE_SRGB;
    return 0;
}

static int s4_ad9984a_g_frame_interval(struct v4l2_subdev *sd,
                                       struct v4l2_subdev_frame_interval *fi)
{
    struct s4_ad9984a_state *state = container_of(sd, struct s4_ad9984a_state, sd);
    fi->interval.numerator = 1;
    fi->interval.denominator = state->framerate ?: 60;
    return 0;
}

static const struct v4l2_subdev_core_ops s4_ad9984a_core_ops = {
    .log_status = v4l2_ctrl_subdev_log_status,
};

static const struct v4l2_subdev_video_ops s4_ad9984a_video_ops = {
    .s_stream = s4_ad9984a_s_stream,
    .g_frame_interval = s4_ad9984a_g_frame_interval,
};

static const struct v4l2_subdev_pad_ops s4_ad9984a_pad_ops = {
    .enum_mbus_code = s4_ad9984a_enum_mbus_code,
    .get_fmt = s4_ad9984a_get_fmt,
};

static const struct v4l2_subdev_ops s4_ad9984a_subdev_ops = {
    .core  = &s4_ad9984a_core_ops,
    .video = &s4_ad9984a_video_ops,
    .pad   = &s4_ad9984a_pad_ops,
};

// Debugfs entry
static struct dentry *telemetry_dir;
static struct dentry *timing_file;

static ssize_t show_timings(struct file *file, char __user *buf,
                            size_t count, loff_t *ppos)
{
    struct s4_ad9984a_state *state = file->private_data;
    char output[256];
    int len = snprintf(output, sizeof(output),
        "hsync_len: %u\nvsync_len: %u\nhbp: %u\nvbp: %u\nhfp: %u\nvfp: %u\n",
        state->hsync_len, state->vsync_len,
        state->hbp, state->vbp,
        state->hfp, state->vfp);

    return simple_read_from_buffer(buf, count, ppos, output, len);
}

static const struct file_operations timing_fops = {
    .owner = THIS_MODULE,
    .read = show_timings,
};

// Create debugfs entries
static void create_debugfs_entries(struct s4_ad9984a_state *state)
{
    telemetry_dir = debugfs_create_dir("s4_ad9984a", NULL);
    timing_file = debugfs_create_file("timings", 0444, telemetry_dir, state, &timing_fops);
}

// Probe function
static int s4_ad9984a_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct s4_ad9984a_state *state;
    struct v4l2_subdev *sd;

    state = devm_kzalloc(&client->dev, sizeof(*state), GFP_KERNEL);
    if (!state)
        return -ENOMEM;
																				 

    sd = &state->sd;
    v4l2_i2c_subdev_init(sd, client, &s4_ad9984a_subdev_ops);
    sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
    state->client = client;

    v4l2_ctrl_handler_init(&state->ctrl_handler, 4);
    v4l2_ctrl_new_std(&state->ctrl_handler, NULL, V4L2_CID_WIDTH, 0, 8192, 1, 640);
    v4l2_ctrl_new_std(&state->ctrl_handler, NULL, V4L2_CID_HEIGHT, 0, 8192, 1, 480);
    v4l2_ctrl_new_std(&state->ctrl_handler, NULL, V4L2_CID_FRAMERATE, 1, 240, 1, 60);
    v4l2_ctrl_new_std(&state->ctrl_handler, &s4_ctrl_ops, V4L2_CID_S4_ENABLE_DEBUG, 0, 1, 1, 0);
    sd->ctrl_handler = &state->ctrl_handler;

    state->pad.flags = MEDIA_PAD_FL_SOURCE;
    sd->entity.function = MEDIA_ENT_F_CAM_SENSOR;
    media_entity_pads_init(&sd->entity, 1, &state->pad);

    create_debugfs_entries(state);

    dev_info(&client->dev, "AD9984A VGA decoder initialized\n");
	
	media_entity_pads_init(&sd->entity, 1, &state->pad);
	return v4l2_async_register_subdev(sd);

    return 0;
}

static int s4_ad9984a_remove(struct i2c_client *client)
{
    return 0;
}

static const struct of_device_id s4_ad9984a_of_match[] = {
    { .compatible = "analogdevices,ad9984a" },
    { }
};
MODULE_DEVICE_TABLE(of, s4_ad9984a_of_match);

static struct i2c_driver s4_ad9984a_i2c_driver = {
    .driver = {
        .name = "s4_ad9984a",
    },
    .probe = s4_ad9984a_probe
};

module_i2c_driver(s4_ad9984a_i2c_driver);

MODULE_DESCRIPTION("S4-Gadgets AD9984A V4L2 Driver with Telemetry");
MODULE_AUTHOR("Michael Scott <support@s4gadgets.com>");
MODULE_LICENSE("GPL");
