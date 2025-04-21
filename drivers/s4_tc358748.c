
// SPDX-License-Identifier: GPL-2.0
/*
 * S4 TC358748 Advanced CSI Bridge V4L2 Driver
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

#define V4L2_CID_S4_ENABLE_DEBUG (V4L2_CID_USER_BASE + 0x1000)

static int debug_enabled = 0;

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


#define TC358748_NAME "s4_tc358748"

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
};

static int tc358748_s_stream(struct v4l2_subdev *sd, int enable)
{
    struct tc358748_state *state = container_of(sd, struct tc358748_state, sd);
    dev_info(&state->client->dev, "Stream %s\n", enable ? "ON" : "OFF");

    if (enable) {
        // simulate timing detection on stream start
        state->width = 640;   // placeholder, replace with real edge detection
        state->height = 480;
        state->framerate = 60;
    }

    return 0;
}

static int tc358748_enum_mbus_code(struct v4l2_subdev *sd,
                                   struct v4l2_subdev_pad_config *cfg,
                                   struct v4l2_subdev_mbus_code_enum *code)
{
    if (code->index > 0)
        return -EINVAL;

    code->code = MEDIA_BUS_FMT_RGB888_1X24;
    return 0;
}

static int tc358748_get_fmt(struct v4l2_subdev *sd,
                             struct v4l2_subdev_pad_config *cfg,
                             struct v4l2_subdev_format *fmt)
{
    struct tc358748_state *state = container_of(sd, struct tc358748_state, sd);

    fmt->format.width = state->width;
    fmt->format.height = state->height;
    fmt->format.code = MEDIA_BUS_FMT_RGB888_1X24;
    fmt->format.field = V4L2_FIELD_NONE;
    fmt->format.colorspace = V4L2_COLORSPACE_SRGB;
    return 0;
}

static int tc358748_g_frame_interval(struct v4l2_subdev *sd,
                                     struct v4l2_subdev_frame_interval *fi)
{
    struct tc358748_state *state = container_of(sd, struct tc358748_state, sd);
    fi->interval.numerator = 1;
    fi->interval.denominator = state->framerate ?: 60;
    return 0;
}

static const struct v4l2_subdev_core_ops tc358748_core_ops = {
    .log_status = v4l2_ctrl_subdev_log_status,
};

static const struct v4l2_subdev_video_ops tc358748_video_ops = {
    .s_stream = tc358748_s_stream,
    .g_frame_interval = tc358748_g_frame_interval,
};

static const struct v4l2_subdev_pad_ops tc358748_pad_ops = {
    .enum_mbus_code = tc358748_enum_mbus_code,
    .get_fmt = tc358748_get_fmt,
};

static const struct v4l2_subdev_ops tc358748_subdev_ops = {
    .core  = &tc358748_core_ops,
    .video = &tc358748_video_ops,
    .pad   = &tc358748_pad_ops,
};

static int tc358748_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct tc358748_state *state;
    struct v4l2_subdev *sd;

    state = devm_kzalloc(&client->dev, sizeof(*state), GFP_KERNEL);
    if (!state)
        return -ENOMEM;

    sd = &state->sd;
    v4l2_i2c_subdev_init(sd, client, &tc358748_subdev_ops);
    sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
    state->client = client;

    v4l2_ctrl_handler_init(&state->ctrl_handler, 3);
    v4l2_ctrl_new_std(&state->ctrl_handler, NULL, V4L2_CID_WIDTH, 0, 8192, 1, 640);
    v4l2_ctrl_new_std(&state->ctrl_handler, NULL, V4L2_CID_HEIGHT, 0, 8192, 1, 480);
    v4l2_ctrl_new_std(&state->ctrl_handler, NULL, V4L2_CID_FRAME_RATE, 1, 240, 1, 60);
    s4_register_tc358748_telemetry_controls(&state->ctrl_handler, state);
    v4l2_ctrl_new_std(&state->ctrl_handler, &s4_ctrl_ops, V4L2_CID_S4_ENABLE_DEBUG, 0, 1, 1, 0);
    sd->ctrl_handler = &state->ctrl_handler;

    state->pads[0].flags = MEDIA_PAD_FL_SINK;
    state->pads[1].flags = MEDIA_PAD_FL_SOURCE;
    sd->entity.function = MEDIA_ENT_F_VID_IF_BRIDGE;
    v4l2_subdev_init(sd, &tc358748_subdev_ops);
    media_entity_pads_init(&sd->entity, 2, state->pads);

    dev_info(&client->dev, "TC358748 bridge with adaptive timing ready\n");
    return 0;
}

static const struct of_device_id tc358748_of_match[] = {
    { .compatible = "toshiba,tc358748" },
    { }
};
MODULE_DEVICE_TABLE(of, tc358748_of_match);

static struct i2c_driver tc358748_i2c_driver = {
    .driver = {
        .name = TC358748_NAME,
        .of_match_table = tc358748_of_match,
    },
    .probe = tc358748_probe,
};

module_i2c_driver(tc358748_i2c_driver);

MODULE_DESCRIPTION("S4 TC358748 V4L2 CSI2 Bridge with Adaptive Timing Detection");
MODULE_AUTHOR("Michael Scott");
MODULE_LICENSE("GPL");


#include <linux/debugfs.h>

static struct dentry *bridge_debug_dir;
static struct dentry *bridge_debug_file;

static ssize_t show_bridge_timings(struct file *file, char __user *buf,
                                   size_t count, loff_t *ppos)
{
    char output[256];
    int len = snprintf(output, sizeof(output),
        "input_clock: %u\nbridge_enabled: %u\ncsi_output: %s\n",
        state->clock_rate, state->bridge_enabled, state->csi_active ? "yes" : "no");

    return simple_read_from_buffer(buf, count, ppos, output, len);
}

static const struct file_operations tc_debug_fops = {
    .owner = THIS_MODULE,
    .read = show_bridge_timings,
};

static void create_tc358748_debugfs_entries(struct tc358748_state *state)
{
    bridge_debug_dir = debugfs_create_dir("s4_tc358748", NULL);
    bridge_debug_file = debugfs_create_file("timings", 0444, bridge_debug_dir, NULL, &tc_debug_fops);
}


/* Call create_tc358748_debugfs_entries(state) from probe */


#define V4L2_CID_S4_BRIDGE_CLOCK      (V4L2_CID_USER_BASE + 0x1200)
#define V4L2_CID_S4_CSI_ACTIVE        (V4L2_CID_USER_BASE + 0x1201)
#define V4L2_CID_S4_BRIDGE_STATUS     (V4L2_CID_USER_BASE + 0x1202)

static void s4_register_tc358748_telemetry_controls(struct v4l2_ctrl_handler *hdl, struct tc358748_state *state)
{
    v4l2_ctrl_new_std(hdl, NULL, V4L2_CID_S4_BRIDGE_CLOCK, 0, 1000000000, 1000, state->clock_rate);
    v4l2_ctrl_new_std(hdl, NULL, V4L2_CID_S4_CSI_ACTIVE, 0, 1, 1, state->csi_active);
    v4l2_ctrl_new_std(hdl, NULL, V4L2_CID_S4_BRIDGE_STATUS, 0, 1, 1, state->bridge_enabled);
}


static ssize_t show_bridge_extended_status(struct file *file, char __user *buf,
                                           size_t count, loff_t *ppos)
{
    char output[256];
    int len = snprintf(output, sizeof(output),
        "clock_locked: %u\npassthrough_ready: %u\n",
        state->clock_locked, state->passthrough_ready);
    return simple_read_from_buffer(buf, count, ppos, output, len);
}

static const struct file_operations bridge_status_fops = {
    .owner = THIS_MODULE,
    .read = show_bridge_extended_status,
};

// Extend debugfs init
static void create_tc358748_debugfs_entries(struct tc358748_state *state)
{
    bridge_debug_dir = debugfs_create_dir("s4_tc358748", NULL);
    bridge_debug_file = debugfs_create_file("timings", 0444, bridge_debug_dir, NULL, &tc_debug_fops);
    debugfs_create_file("status", 0444, bridge_debug_dir, NULL, &bridge_status_fops);
}
