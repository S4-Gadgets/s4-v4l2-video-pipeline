
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


#define AD9984A_NAME "s4_ad9984a"

struct ad9984a_state {
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

static int ad9984a_read_timing(struct ad9984a_state *state)
{
    // Placeholder: replace with real sync pulse edge detection or sync timer
    state->h_active = 640;
    state->v_active = 480;
    state->framerate = 60;
    return 0;
}

static int ad9984a_s_stream(struct v4l2_subdev *sd, int enable)
{
    struct ad9984a_state *state = container_of(sd, struct ad9984a_state, sd);

    if (enable) {
        ad9984a_read_timing(state);
        dev_info(&state->client->dev, "Detected %ux%u @ %uHz\n",
                 state->h_active, state->v_active, state->framerate);
    }

    return 0;
}

static int ad9984a_enum_mbus_code(struct v4l2_subdev *sd,
                                   struct v4l2_subdev_pad_config *cfg,
                                   struct v4l2_subdev_mbus_code_enum *code)
{
    if (code->index > 0)
        return -EINVAL;

    code->code = MEDIA_BUS_FMT_RGB888_1X24;
    return 0;
}

static int ad9984a_get_fmt(struct v4l2_subdev *sd,
                             struct v4l2_subdev_pad_config *cfg,
                             struct v4l2_subdev_format *fmt)
{
    struct ad9984a_state *state = container_of(sd, struct ad9984a_state, sd);

    fmt->format.width = state->h_active;
    fmt->format.height = state->v_active;
    fmt->format.code = MEDIA_BUS_FMT_RGB888_1X24;
    fmt->format.field = V4L2_FIELD_NONE;
    fmt->format.colorspace = V4L2_COLORSPACE_SRGB;
    return 0;
}

static int ad9984a_g_frame_interval(struct v4l2_subdev *sd,
                                     struct v4l2_subdev_frame_interval *fi)
{
    struct ad9984a_state *state = container_of(sd, struct ad9984a_state, sd);
    fi->interval.numerator = 1;
    fi->interval.denominator = state->framerate ?: 60;
    return 0;
}

static const struct v4l2_subdev_core_ops ad9984a_core_ops = {
    .log_status = v4l2_ctrl_subdev_log_status,
};

static const struct v4l2_subdev_video_ops ad9984a_video_ops = {
    .s_stream = ad9984a_s_stream,
    .g_frame_interval = ad9984a_g_frame_interval,
};

static const struct v4l2_subdev_pad_ops ad9984a_pad_ops = {
    .enum_mbus_code = ad9984a_enum_mbus_code,
    .get_fmt = ad9984a_get_fmt,
};

static const struct v4l2_subdev_ops ad9984a_subdev_ops = {
    .core  = &ad9984a_core_ops,
    .video = &ad9984a_video_ops,
    .pad   = &ad9984a_pad_ops,
};

static int ad9984a_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct ad9984a_state *state;
    struct v4l2_subdev *sd;

    state = devm_kzalloc(&client->dev, sizeof(*state), GFP_KERNEL);
    if (!state)
        return -ENOMEM;

    sd = &state->sd;
    v4l2_i2c_subdev_init(sd, client, &ad9984a_subdev_ops);
    sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
    state->client = client;

    v4l2_ctrl_handler_init(&state->ctrl_handler, 3);
    v4l2_ctrl_new_std(&state->ctrl_handler, NULL, V4L2_CID_WIDTH, 0, 8192, 1, 640);
    v4l2_ctrl_new_std(&state->ctrl_handler, NULL, V4L2_CID_HEIGHT, 0, 8192, 1, 480);
    v4l2_ctrl_new_std(&state->ctrl_handler, NULL, V4L2_CID_FRAME_RATE, 1, 240, 1, 60);
    s4_register_telemetry_controls(&state->ctrl_handler, state);
    v4l2_ctrl_new_std(&state->ctrl_handler, &s4_ctrl_ops, V4L2_CID_S4_ENABLE_DEBUG, 0, 1, 1, 0);
    sd->ctrl_handler = &state->ctrl_handler;

    state->pad.flags = MEDIA_PAD_FL_SOURCE;
    sd->entity.function = MEDIA_ENT_F_CAM_SENSOR;
    media_entity_pads_init(&sd->entity, 1, &state->pad);

    dev_info(&client->dev, "AD9984A VGA decoder initialized\n");
    return 0;
}

static const struct of_device_id ad9984a_of_match[] = {
    { .compatible = "analogdevices,ad9984a" },
    { }
};
MODULE_DEVICE_TABLE(of, ad9984a_of_match);

static struct i2c_driver ad9984a_i2c_driver = {
    .driver = {
        .name = AD9984A_NAME,
        .of_match_table = ad9984a_of_match,
    },
    .probe = ad9984a_probe,
};

module_i2c_driver(ad9984a_i2c_driver);

MODULE_DESCRIPTION("S4 AD9984A V4L2 Analog VGA Decoder with Adaptive Timing");
MODULE_AUTHOR("Michael Scott");
MODULE_LICENSE("GPL");


#include <linux/debugfs.h>

static struct dentry *telemetry_dir;
static struct dentry *timing_file;

static ssize_t show_timings(struct file *file, char __user *buf,
                            size_t count, loff_t *ppos)
{
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

static void create_debugfs_entries(struct ad9984a_state *state)
{
    telemetry_dir = debugfs_create_dir("s4_ad9984a", NULL);
    timing_file = debugfs_create_file("timings", 0444, telemetry_dir, NULL, &timing_fops);
}


/* Call create_debugfs_entries(state) from probe */


#define V4L2_CID_S4_HSYNC_LEN     (V4L2_CID_USER_BASE + 0x1100)
#define V4L2_CID_S4_VSYNC_LEN     (V4L2_CID_USER_BASE + 0x1101)
#define V4L2_CID_S4_HBP           (V4L2_CID_USER_BASE + 0x1102)
#define V4L2_CID_S4_VBP           (V4L2_CID_USER_BASE + 0x1103)
#define V4L2_CID_S4_HFP           (V4L2_CID_USER_BASE + 0x1104)
#define V4L2_CID_S4_VFP           (V4L2_CID_USER_BASE + 0x1105)
#define V4L2_CID_S4_FRAMERATE     (V4L2_CID_USER_BASE + 0x1106)
#define V4L2_CID_S4_PIXELCLOCK    (V4L2_CID_USER_BASE + 0x1107)

static void s4_register_telemetry_controls(struct v4l2_ctrl_handler *hdl, struct ad9984a_state *state)
{
    v4l2_ctrl_new_std(hdl, NULL, V4L2_CID_S4_HSYNC_LEN, 0, 8192, 1, state->hsync_len);
    v4l2_ctrl_new_std(hdl, NULL, V4L2_CID_S4_VSYNC_LEN, 0, 8192, 1, state->vsync_len);
    v4l2_ctrl_new_std(hdl, NULL, V4L2_CID_S4_HBP, 0, 8192, 1, state->hbp);
    v4l2_ctrl_new_std(hdl, NULL, V4L2_CID_S4_VBP, 0, 8192, 1, state->vbp);
    v4l2_ctrl_new_std(hdl, NULL, V4L2_CID_S4_HFP, 0, 8192, 1, state->hfp);
    v4l2_ctrl_new_std(hdl, NULL, V4L2_CID_S4_VFP, 0, 8192, 1, state->vfp);
    v4l2_ctrl_new_std(hdl, NULL, V4L2_CID_S4_FRAMERATE, 0, 240, 1, state->fps);
    v4l2_ctrl_new_std(hdl, NULL, V4L2_CID_S4_PIXELCLOCK, 0, 1000000000, 1000, state->pixelclock);
}
