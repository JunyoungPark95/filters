#pragma once
#include "filters.h"

float* Filter::BoxFiltering(float* inputColor, int xres, int yres) { 	// Box filter
	float invN = (1.0f / ((float)KernelSize * KernelSize));
	float *estColor = new float[yres * xres * 3];
	int iter = 0;
	// estColor = sum(inputColor * 1/ N)
	for (int cy = 0; cy < yres; ++cy) {
		for (int cx = 0; cx < xres; ++cx) {
			//create kernel
			float EstRGB[3] = { 0.0f, };
			
			for (int ny = cy - KernelRadius; ny <= cy + KernelRadius; ++ny) {
				for (int nx = cx - KernelRadius; nx <= cx + KernelRadius; ++nx) {
					
					int XIndex = nx < 0 ? -nx : nx;
					int YIndex = ny < 0 ? -ny : ny;

					XIndex = XIndex > 1279 ? (2 * xres) - XIndex - 2: XIndex;
					YIndex = YIndex > 719 ? (2 * yres) - YIndex - 2 : YIndex;

					if (XIndex >= 1280)
						std::cout << "warning" << "XIndex :  "  << XIndex << std::endl;

					int Index = xres * YIndex + XIndex;

					EstRGB[0] += inputColor[3 * Index]; //R
					EstRGB[1] += inputColor[3 * Index + 1]; //G
					EstRGB[2] += inputColor[3 * Index + 2]; //B
					iter++;
				}
			}
			int index = xres * cy + cx;

			estColor[3 * index] = invN * EstRGB[0];
			estColor[3 * index + 1] = invN * EstRGB[1];
			estColor[3 * index + 2] = invN * EstRGB[2];

		}
	}
	return estColor;
} // BoxFiltering