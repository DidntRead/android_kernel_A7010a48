#include <primary_display.h>
#include "include/ddp_gamma.h"
#include <ddp_reg.h>
#include <ddp_drv.h>

struct pp_data {
	int red;
	int green;
	int blue;
	int minimum;
	int enable;
	int invert;
	int sat;
	int hue;
	int val;
	int cont;
};

#define MAX_LUT_SCALE 2000
#define PROGRESSION_SCALE 1000
