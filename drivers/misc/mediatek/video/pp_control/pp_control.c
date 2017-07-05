#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/module.h>
#include "pp_control.h"

static void update_pp(struct pp_data *pp_data) {
	int i, gammutR, gammutG, gammutB, ret, bypass;
	DISP_GAMMA_LUT_T *gamma;
	
	if (pp_data->enable == 1) {
	bypass = 0;
	} else {
	bypass = 1;	
	}

	DISP_REG_MASK(NULL, DISP_REG_GAMMA_CFG, bypass, 0x1);

	if (pp_data->red > pp_data->minimum) {
	pp_data->red = pp_data->minimum;	
	}
	if (pp_data->green > pp_data->minimum) {
	pp_data->green = pp_data->minimum;	
	}
	if (pp_data->blue > pp_data->minimum) {
	pp_data->blue = pp_data->minimum;	
	}

	gamma = kzalloc(sizeof(DISP_GAMMA_LUT_T), GFP_KERNEL);
	gamma->hw_id = 0;

	for (i = 0; i < 512; i++) {
		gammutR = i * pp_data->red / PROGRESSION_SCALE;
		gammutG = i * pp_data->green / PROGRESSION_SCALE;
		gammutB = i * pp_data->blue / PROGRESSION_SCALE;

		gamma->lut[i] = GAMMA_ENTRY(gammutR, gammutG, gammutB);
	}

	ret = primary_display_user_cmd(DISP_IOCTL_SET_GAMMALUT, (unsigned long)gamma);
	kfree(gamma);
}

static ssize_t rgb_show(struct device *dev, struct device_attribute *attr,
								char *buf)
{
	struct pp_data *pp_data = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "%d %d %d\n", pp_data->red, pp_data->green, pp_data->blue);
}

static ssize_t rgb_store(struct device *dev, struct device_attribute *attr,
const char *buf, size_t count) {
	int r, g, b, ret;
	struct pp_data *pp_data = dev_get_drvdata(dev);

	ret = sscanf(buf, "%d %d %d", &r, &g, &b);

		if ((ret != 3) || (r < 1 || r > 2000) ||
		(g < 1 || g > 2000) || (b < 1 || b > 2000)) {
		return -EINVAL;
		}

	pp_data->red = r;
	pp_data->green = g;
	pp_data->blue = b;
	update_pp(pp_data);
	return count;
}

static ssize_t min_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int min, ret;
	struct pp_data *pp_data = dev_get_drvdata(dev);

	ret = kstrtoint(buf, 10, &min);
	if ((ret) || (min < 1 || min > 2000))
		return -EINVAL;

	pp_data->minimum = min;
	update_pp(pp_data);
	return count;
}

static ssize_t min_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct pp_data *pp_data = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", pp_data->minimum);
}

static ssize_t enable_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int enable, ret;
	struct pp_data *pp_data = dev_get_drvdata(dev);

	ret = kstrtoint(buf, 10, &enable);
	if ((ret) || (enable != 0 && enable != 1) ||
		(pp_data->enable == enable))
		return -EINVAL;

	pp_data->enable = enable;
	update_pp(pp_data);
	return count;
}

static ssize_t enable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct pp_data *pp_data = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", pp_data->enable);
}

static DEVICE_ATTR(rgb, S_IWUSR | S_IRUGO, rgb_show, rgb_store);
static DEVICE_ATTR(min, S_IWUSR | S_IRUGO, min_show, min_store);
static DEVICE_ATTR(enable, S_IWUSR | S_IRUGO, enable_show,
	enable_store);

static int pp_control_probe(struct platform_device *pdev)
{
	int ret;
	struct pp_data *pp_data;

	pp_data = devm_kzalloc(&pdev->dev, sizeof(*pp_data), GFP_KERNEL);
	if (!pp_data) {
		pr_err("%s: failed to allocate memory for pp_data\n",
			__func__);
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, pp_data);

	pp_data->enable = 1;
	pp_data->red = MAX_LUT_SCALE;
	pp_data->green = MAX_LUT_SCALE;
	pp_data->blue = MAX_LUT_SCALE;
	pp_data->minimum = 1;


	ret = device_create_file(&pdev->dev, &dev_attr_rgb);
	ret |= device_create_file(&pdev->dev, &dev_attr_min);
	ret |= device_create_file(&pdev->dev, &dev_attr_enable);

	if (ret) {
		pr_err("%s: unable to create sysfs entries\n", __func__);
		return ret;
	}

	return 0;
}

static int pp_control_remove(struct platform_device *pdev)
{
	device_remove_file(&pdev->dev, &dev_attr_rgb);
	device_remove_file(&pdev->dev, &dev_attr_min);
	device_remove_file(&pdev->dev, &dev_attr_enable);

	return 0;
}

static struct platform_driver pp_control_driver = {
	.probe = pp_control_probe,
	.remove = pp_control_remove,
	.driver = {
		.name = "pp_control",
	},
};

static struct platform_device pp_control_device = {
	.name = "pp_control",
};

static int __init pp_control_init(void)
{
	if (platform_driver_register(&pp_control_driver))
		return -ENODEV;

	if (platform_device_register(&pp_control_device))
		return -ENODEV;

	pr_info("%s: registered\n", __func__);

	return 0;
}

static void __exit pp_control_exit(void)
{
	platform_driver_unregister(&pp_control_driver);	
	platform_device_unregister(&pp_control_device);
}

module_init(pp_control_init);
module_exit(pp_control_exit);
MODULE_LICENSE("GPL and additional rights");
MODULE_AUTHOR("DarkBlood <gabro2003@gmail.com>");
MODULE_DESCRIPTION("Post processing control driver");
