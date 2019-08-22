#pragma once
#include "filters.h"

float* Filter::BoxFiltering(float* inputColor, int xres, int yres) { 	// Box filter
	float invN = (1.0f / ((float)KernelSize * KernelSize));
	float *estColor = new float[yres * xres * 3];
	int idx = 0;
	// estColor = sum(inputColor * 1/ N)
	for (int c2 = 0; c2 < yres; ++c2) {
		for (int c1 = 0; c1 < xres; ++c1) {


		}
	}
	return estColor;
} // BoxFiltering