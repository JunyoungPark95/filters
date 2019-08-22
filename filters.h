#pragma once
#include "Image.h"

class Filter {
public:
	float* BoxFiltering(float *inputColor, int xres, int yres);
}; // BoxFilter