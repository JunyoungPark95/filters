#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "config.h"
#include "filterCUDA.h"

#include <time.h>

#define MAX(a, b) a > b ? a : b
#define MIN(a, b) a < b ? a : b 

__global__ void kernel_box_filter(float *estColor, float *inputColor, int xres, int yres, float *time) {
	clock_t start, end;
	double result;

	start = clock(); // running time evaluation start
	const int cx = blockDim.x * blockIdx.x + threadIdx.x;
	const int cy = blockDim.y * blockIdx.y + threadIdx.y;

	if (cx >= xres || cy >= yres) {
		return;
	}
	
	int index = cy * xres + cx;

	int sampleCount = 0;
	float color[3] = { 0.0f, };
	int sx = MAX(cx - KernelRadius, 0); // adaptively controll the size of kernel entirely 4 cases
	int sy = MAX(cy - KernelRadius, 0);
	int ex = MIN(cx + KernelRadius, xres - 1);
	int ey = MIN(cy + KernelRadius, yres - 1);
	for (int ny = sy; ny <= ey; ++ny) { // kernel
		for (int nx = sx; nx <= ex; ++nx) {
			color[0] += inputColor[3 * (xres * ny + nx) + 0];
			color[1] += inputColor[3 * (xres * ny + nx) + 1];
			color[2] += inputColor[3 * (xres * ny + nx) + 2];
			sampleCount++;
		}
	}
	estColor[3 * index + 0] = 1.f / (float)sampleCount * color[0];
	estColor[3 * index + 1] = 1.f / (float)sampleCount * color[1];
	estColor[3 * index + 2] = 1.f / (float)sampleCount * color[2];

	end = clock();
	result = (float)(end - start);

	time[1] = (result) / CLOCKS_PER_SEC;
}

extern "C" void BoxFiltering(float *outputColor, float* inputColor, int xres, int yres, float *time) { 

	float invN = (1.0f / ((float)KernelSize * KernelSize));
	float *estColor = new float[yres * xres * 3]; // estColor = sum(inputColor * 1/ N)

	// Cuda mem allocation
	// Host mem cpy
	float *d_input_color, *d_output_color;

	const int memSize = sizeof(float) * 3 * xres * yres;

	cudaMalloc((void **)&d_input_color, memSize);
	cudaMalloc((void **)&d_output_color, memSize);

	cudaMemcpy(&d_input_color[0], &inputColor[0], memSize, cudaMemcpyHostToDevice);

	// kernel function 
	// for loop => parallel execution

	const int blockDim = 16;
	dim3 block(blockDim, blockDim);
	dim3 grid((xres + block.x - 1) / block.x, (yres + block.y - 1) / block.y);

	kernel_box_filter<<<grid, block>>>(d_output_color, d_input_color, xres, yres, time);

	cudaMemcpy(&outputColor[0], &d_output_color[0], memSize, cudaMemcpyDeviceToHost);

	cudaFree(d_input_color);
	cudaFree(d_output_color);
}