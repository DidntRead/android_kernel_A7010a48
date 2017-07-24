/*
 * Copyright � 2017, DarkBlood <gabro2003@gmail.com>
 *
 * Post-processing controller for MTK
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * Please preserve this licence and driver name if you implement this 
 * anywhere else.
 *
 */

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

#define pp_control_version 1
#define pp_control_subversion 6

static void update_pp(struct pp_data *pp_data) {
	int i, gammutR, gammutG, gammutB;
	unsigned char h_series[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned int u4Temp = 0;
	int index = 0;
	DISP_GAMMA_LUT_T *gamma;
	
	if (pp_data->enable) {
		_color_reg_mask(NULL, DISP_COLOR_CFG_MAIN + offset, (0 << 7), 0x00000080);
		_color_reg_set(NULL, DISP_COLOR_START + offset, 0x00000001);
		_color_reg_set(NULL, DISP_COLOR_CM1_EN + offset, 0x01);
		_color_reg_set(NULL, DISP_COLOR_CM2_EN + offset, 0x11);
	} else {
		_color_reg_set(NULL, DISP_COLOR_CM1_EN + offset, 0);
		_color_reg_set(NULL, DISP_COLOR_CM2_EN + offset, 0);
		_color_reg_mask(NULL, DISP_COLOR_CFG_MAIN + offset, (1 << 7), 0x00000080);
		_color_reg_set(NULL, DISP_COLOR_START + offset, 0x00000003);
	}


	if (pp_data->red < pp_data->minimum) {
	pp_data->red = pp_data->minimum;	
	}
	if (pp_data->green < pp_data->minimum) {
	pp_data->green = pp_data->minimum;	
	}
	if (pp_data->blue < pp_data->minimum) {
	pp_data->blue = pp_data->minimum;	
	}

	gamma = kzalloc(sizeof(DISP_GAMMA_LUT_T), GFP_KERNEL);
	gamma->hw_id = 0;
	
	if (pp_data->enable) {
	for (i = 0; i < 512; i++) {
		gammutR = i * pp_data->red / PROGRESSION_SCALE;
		gammutG = i * pp_data->green / PROGRESSION_SCALE;
		gammutB = i * pp_data->blue / PROGRESSION_SCALE;

		gamma->lut[i] = GAMMA_ENTRY(gammutR, gammutG, gammutB);
	} } else {
	for (i = 0; i < 512; i++) {
		gammutR = i * 2000 / PROGRESSION_SCALE;
		gammutG = i * 2000 / PROGRESSION_SCALE;
		gammutB = i * 2000 / PROGRESSION_SCALE;

		gamma->lut[i] = GAMMA_ENTRY(gammutR, gammutG, gammutB);
	}
	}

	printk("[pp_control]r: %d g:%d b:%d enable:%d \n", pp_data->red, pp_data->green, pp_data->blue, pp_data->enable);
	primary_display_user_cmd(DISP_IOCTL_SET_GAMMALUT, (unsigned long)gamma);
	kfree(gamma);	
	_color_reg_set(NULL, DISP_COLOR_G_PIC_ADJ_MAIN_2,
	pp_data->sat);
	_color_reg_set(NULL, DISP_COLOR_G_PIC_ADJ_MAIN_1,
	( pp_data->brightness << 16 ) | pp_data->cont);
	/*	
	for (index = 0; index < 3; index++) {
		h_series[index] = pp_data->hue_purple;
	}
	for (index = 3; index < 11; index++) {
		h_series[index] = pp_data->hue_skin;
	}
	for (index = 11; index < 17; index++) {
		h_series[index] = pp_data->hue_grass;
	}
	for (index = 17; index < 20; index++) {
		h_series[index] = pp_data->hue_sky;
	}
	*/
	for (index = 0; index < 20; index++) {
		h_series[index] = pp_data->hue;
	}
		
	for (index = 0; index < 5; index++) {
		u4Temp = (h_series[4 * index]) +
		    (h_series[4 * index + 1] << 8) +
		    (h_series[4 * index + 2] << 16) + (h_series[4 * index + 3] << 24);
		_color_reg_set(NULL, DISP_COLOR_LOCAL_HUE_CD_0 + offset + 4 * index, u4Temp);
	}

	color_trigger_refresh(DISP_MODULE_COLOR0);
	
}

static ssize_t min_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int min, ret;
	struct pp_data *pp_data = dev_get_drvdata(dev);

	ret = sscanf(buf, "%d", &min);
	if ((!ret) || (min < 1 || min > 2000))
		return -EINVAL;

	if (primary_display_is_sleepd()) {
	return -EINVAL;
	}

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

static ssize_t green_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int green, ret;
	struct pp_data *pp_data = dev_get_drvdata(dev);

	ret = sscanf(buf, "%d", &green);
	if ((!ret) || (green > 2000))
		return -EINVAL;

	if (primary_display_is_sleepd()) {
	return -EINVAL;
	}

	pp_data->green = green;
	update_pp(pp_data);
	return count;
}

static ssize_t ccor_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct pp_data *pp_data = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%u %u %u %u %u %u %u %u %u\n", pp_data->coef[0][0], pp_data->coef[0][1], pp_data->coef[0][2], pp_data->coef[1][0], pp_data->coef[1][1], pp_data->coef[1][2], pp_data->coef[2][0], pp_data->coef[2][1], pp_data->coef[2][2]);
}

static ssize_t ccor_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int one,two,three,four,five,six,seven,eight,nine, ret;
	struct pp_data *pp_data = dev_get_drvdata(dev);
	#define CCORR_REG(base, idx) (base + (idx) * 4 + 0x80)
	const unsigned long ccorr_base = DISPSYS_CCORR_BASE;
	ret = sscanf(buf, "%u %u %u %u %u %u %u %u %u", &one, &two, &three, &four, &five, &six, &seven, &eight, &nine);

	if (primary_display_is_sleepd()) {
	return -EINVAL;
	}

	pp_data->coef[0][0] = one;
	pp_data->coef[0][1] = two;
	pp_data->coef[0][2] = three;
	pp_data->coef[1][0] = four;
	pp_data->coef[1][1] = five;
	pp_data->coef[1][2] = six;
	pp_data->coef[2][0] = seven;
	pp_data->coef[2][1] = eight;
	pp_data->coef[2][2] = nine;

	DISP_REG_SET(NULL, DISP_REG_CCORR_EN, 1);
	DISP_REG_MASK(NULL, DISP_REG_CCORR_CFG, 0x2, 0x2);

	DISP_REG_SET(NULL, CCORR_REG(ccorr_base, 0), //RED
		     ((one << 16) | (two)));
	DISP_REG_SET(NULL, CCORR_REG(ccorr_base, 1),
		     ((three << 16) | (four)));
	DISP_REG_SET(NULL, CCORR_REG(ccorr_base, 2),  //GREEN
		     ((five << 16) | (six)));
	DISP_REG_SET(NULL, CCORR_REG(ccorr_base, 3),	
		     ((seven << 16) | (eight)));
	DISP_REG_SET(NULL, CCORR_REG(ccorr_base, 4), (nine << 16));  //BLUE

	return count;
}

static ssize_t green_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct pp_data *pp_data = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", pp_data->green);
}

static ssize_t blue_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int blue, ret;
	struct pp_data *pp_data = dev_get_drvdata(dev);

	ret = sscanf(buf, "%d", &blue);
	if ((!ret) || (blue > 2000))
		return -EINVAL;

	if (primary_display_is_sleepd()) {
	return -EINVAL;
	}

	pp_data->blue = blue;
	update_pp(pp_data);
	return count;
}

static ssize_t blue_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct pp_data *pp_data = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", pp_data->blue);
}

static ssize_t red_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int red, ret;
	struct pp_data *pp_data = dev_get_drvdata(dev);

	ret = sscanf(buf, "%d", &red);
	if ((!ret) || (red > 2000))
		return -EINVAL;

	if (primary_display_is_sleepd()) {
	return -EINVAL;
	}

	pp_data->red = red;
	update_pp(pp_data);
	return count;
}

static ssize_t red_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct pp_data *pp_data = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", pp_data->red);
}

static ssize_t enable_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int enable, ret;
	struct pp_data *pp_data = dev_get_drvdata(dev);

	ret = sscanf(buf, "%d", &enable);
	if ((!ret) || (enable != 0 && enable != 1) ||
		(pp_data->enable == enable))
		return -EINVAL;

	if (primary_display_is_sleepd()) {
	return -EINVAL;
	}

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

static ssize_t sat_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int sat, ret;
	struct pp_data *pp_data = dev_get_drvdata(dev);

	ret = sscanf(buf, "%d", &sat);

	if (primary_display_is_sleepd()) {
	return -EINVAL;
	}
	
	pp_data->sat = sat;
	update_pp(pp_data);
	return count;
}

static ssize_t sat_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct pp_data *pp_data = dev_get_drvdata(dev);
	pp_data->sat = DISP_REG_GET(DISP_COLOR_G_PIC_ADJ_MAIN_2);
	return scnprintf(buf, PAGE_SIZE, "%d\n", pp_data->sat);
}

static ssize_t cont_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int cont, ret;
	struct pp_data *pp_data = dev_get_drvdata(dev);

	ret = sscanf(buf, "%d", &cont);

	if (primary_display_is_sleepd()) {
	return -EINVAL;
	}
	
	pp_data->cont = cont;
	update_pp(pp_data);
	return count;
}

static ssize_t cont_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct pp_data *pp_data = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "%d\n", pp_data->cont);
}

static ssize_t brig_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int brig, ret;
	struct pp_data *pp_data = dev_get_drvdata(dev);

	ret = sscanf(buf, "%d", &brig);

	if (primary_display_is_sleepd()) {
	return -EINVAL;
	}
	
	pp_data->brightness = brig;
	update_pp(pp_data);
	return count;
}

static ssize_t hue_show(struct device *dev, struct device_attribute *attr,
								char *buf)
{
	struct pp_data *pp_data = dev_get_drvdata(dev);
	//return scnprintf(buf, PAGE_SIZE, "%d %d %d %d\n", pp_data->hue_purple, pp_data->hue_skin, pp_data->hue_grass, pp_data->hue_sky);
	return scnprintf(buf, PAGE_SIZE, "%d\n", pp_data->hue);
}

static ssize_t hue_store(struct device *dev, struct device_attribute *attr,
const char *buf, size_t count) {
	//int purp, skin, grass, sky, ret;
	int hue, ret;	
	struct pp_data *pp_data = dev_get_drvdata(dev);

	//ret = sscanf(buf, "%d %d %d %d", &purp, &skin, &grass, &sky);
	ret = sscanf(buf, "%d", &hue);

	if (primary_display_is_sleepd()) {
	return -EINVAL;
	}

	pp_data->hue = hue;
	update_pp(pp_data);
	return count;
}

static ssize_t brig_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct pp_data *pp_data = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "%d\n", pp_data->brightness);
}

static ssize_t version_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "v%d_%d\n", pp_control_version, pp_control_subversion);
}

static DEVICE_ATTR(hue, S_IWUSR | S_IRUGO, hue_show, hue_store);
static DEVICE_ATTR(sat, S_IWUSR | S_IRUGO, sat_show, sat_store);
static DEVICE_ATTR(green, S_IWUSR | S_IRUGO, green_show, green_store);
static DEVICE_ATTR(blue, S_IWUSR | S_IRUGO, blue_show, blue_store);
static DEVICE_ATTR(red, S_IWUSR | S_IRUGO, red_show, red_store);
static DEVICE_ATTR(brightness, S_IWUSR | S_IRUGO, brig_show, brig_store);
static DEVICE_ATTR(cont, S_IWUSR | S_IRUGO, cont_show, cont_store);
static DEVICE_ATTR(min, S_IWUSR | S_IRUGO, min_show, min_store);
static DEVICE_ATTR(ccor, S_IWUSR | S_IRUGO, ccor_show, ccor_store);
static DEVICE_ATTR(version, S_IWUSR | S_IRUGO, version_show, NULL);
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
	pp_data->sat = DISP_REG_GET(DISP_COLOR_G_PIC_ADJ_MAIN_2);
	pp_data->cont = 128;
	pp_data->brightness = 1024;
	pp_data->coef[0][0] = 1024;
	pp_data->coef[0][1] = 0;
	pp_data->coef[0][2] = 0;
	pp_data->coef[1][0] = 0;
	pp_data->coef[1][1] = 1024;
	pp_data->coef[1][2] = 0;
	pp_data->coef[2][0] = 0;
	pp_data->coef[2][1] = 0;
	pp_data->coef[2][2] = 1024;
	/*	
	pp_data->hue_purple = 128;
	pp_data->hue_skin = 128;
	pp_data->hue_grass = 128;
	pp_data->hue_sky = 128;
	*/
	pp_data->hue = 128;

	ret = device_create_file(&pdev->dev, &dev_attr_min);
	ret |= device_create_file(&pdev->dev, &dev_attr_cont);
	ret |= device_create_file(&pdev->dev, &dev_attr_enable);
	ret |= device_create_file(&pdev->dev, &dev_attr_sat);
	ret |= device_create_file(&pdev->dev, &dev_attr_version);
	ret |= device_create_file(&pdev->dev, &dev_attr_brightness);
	ret |= device_create_file(&pdev->dev, &dev_attr_hue);
	ret |= device_create_file(&pdev->dev, &dev_attr_blue);
	ret |= device_create_file(&pdev->dev, &dev_attr_green);
	ret |= device_create_file(&pdev->dev, &dev_attr_red);
	ret |= device_create_file(&pdev->dev, &dev_attr_ccor);



	if (ret) {
		pr_err("%s: unable to create sysfs entries\n", __func__);
		return ret;
	}

	return 0;
}

static int pp_control_remove(struct platform_device *pdev)
{
	device_remove_file(&pdev->dev, &dev_attr_min);
	device_remove_file(&pdev->dev, &dev_attr_enable);
	device_remove_file(&pdev->dev, &dev_attr_sat);
	device_remove_file(&pdev->dev, &dev_attr_cont);
	device_remove_file(&pdev->dev, &dev_attr_version);
	device_remove_file(&pdev->dev, &dev_attr_brightness);
	device_remove_file(&pdev->dev, &dev_attr_hue);
	device_remove_file(&pdev->dev, &dev_attr_blue);
	device_remove_file(&pdev->dev, &dev_attr_green);
	device_remove_file(&pdev->dev, &dev_attr_red);
	device_remove_file(&pdev->dev, &dev_attr_ccor);





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
