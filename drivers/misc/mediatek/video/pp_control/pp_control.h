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

#include <primary_display.h>
#include "include/ddp_gamma.h"
#include <ddp_reg.h>
#include <ddp_drv.h>
#include <primary_display.h>
#include <ddp_drv.h>
#include <ddp_color.h>

#define MAX_LUT_SCALE 2000
#define PROGRESSION_SCALE 1000
#define offset (0)

struct pp_data {
	int red;
	int green;
	int blue;
	int minimum;
	int enable;
	int invert;
	int sat;
	int hue;
	int cont;
	int brightness;
};

extern void color_trigger_refresh(DISP_MODULE_ENUM module);
extern void _color_reg_set(void *__cmdq, unsigned long addr,
			   unsigned int value);
extern void _color_reg_mask(void *__cmdq, unsigned long addr,
			    unsigned int value, unsigned int mask);
