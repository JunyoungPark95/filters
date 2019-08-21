#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <algorithm>
#include <iostream>
#include <fstream>

#include <ImfInputFile.h>
#include <ImfChannelList.h>
#include <ImfFrameBuffer.h>
#include <ImfRgbaFile.h>
#include <half.h>

#include "Config.h"

using namespace Imf;
using namespace Imath;

static bool ReadExrRes(const std::string filename, int& xres, int& yres) {
	try {
		InputFile file(filename.c_str());
		Box2i dw = file.header().dataWindow();
		xres = dw.max.x - dw.min.x + 1;
		yres = dw.max.y - dw.min.y + 1;
	}
	catch (const std::exception& e) {
		fprintf(stderr, "Unable to read image file \"%s\": %s", filename, e.what());
		return NULL;
	}
}

template<class T>
static T* ReadExr(const std::string filename, T*& rgb, int channel = 3) { // 포인터의 참조 연산자를 사용 T, float RGB
	try {
		InputFile file(filename.c_str());
		Box2i dw = file.header().dataWindow();
		int xres = dw.max.x - dw.min.x + 1;
		int yres = dw.max.y - dw.min.y + 1;

		half* hrgba = new half[4 * xres * yres];

		hrgba -= 4 * (dw.min.x + dw.min.y * xres);
		FrameBuffer frameBuffer;
		frameBuffer.insert("R", Slice(HALF, (char*)hrgba,
			4 * sizeof(half), xres * 4 * sizeof(half), 1, 1, 0.0));
		frameBuffer.insert("G", Slice(HALF, (char*)hrgba + sizeof(half),
			4 * sizeof(half), xres * 4 * sizeof(half), 1, 1, 0.0));
		frameBuffer.insert("B", Slice(HALF, (char*)hrgba + 2 * sizeof(half),
			4 * sizeof(half), xres * 4 * sizeof(half), 1, 1, 0.0));
		frameBuffer.insert("A", Slice(HALF, (char*)hrgba + 3 * sizeof(half),
			4 * sizeof(half), xres * 4 * sizeof(half), 1, 1, 1.0));

		file.setFrameBuffer(frameBuffer);
		file.readPixels(dw.min.y, dw.max.y);
		hrgba += 4 * (dw.min.x + dw.min.y * xres);

		rgb = new T[xres * yres * channel];

		for (int i = 0; i < xres * yres; ++i) {
			for (int j = 0; j < channel; ++j) {
				rgb[i * channel + j] = (T)hrgba[i * 4 + j];
			}
		}

		delete[] hrgba;
	}
	catch (const std::exception& e) {
		fprintf(stderr, "Unable to read image file \"%s\": %s", filename, e.what());
		return NULL;
	}

	return rgb;
}

template<class T>
static void WriteExr(const std::string filename, T* pixel, int xres, int yres, int channel = 3, const float scale = 1.f) {

	Rgba* hrgba = new Rgba[xres * yres];

	for (int i = 0; i < xres * yres; ++i) {

		float rgb[3];

		rgb[0] = scale * (float)pixel[i * channel + 0 % channel];
		rgb[1] = scale * (float)pixel[i * channel + 1 % channel];
		rgb[2] = scale * (float)pixel[i * channel + 2 % channel];
		hrgba[i] = Rgba(rgb[0], rgb[1], rgb[2], 1.0f);
	}

	Box2i displayWindow(V2i(0, 0), V2i(xres - 1, yres - 1));
	Box2i dataWindow = displayWindow;

	RgbaOutputFile file(filename.c_str(), displayWindow, dataWindow, WRITE_RGBA);
	file.setFrameBuffer(hrgba, 1, xres);
	try {
		file.writePixels(yres);
	}
	catch (const std::exception& e) {
		fprintf(stderr, "Unable to write image file \"%s\": %s", filename,
			e.what());
	}

	delete[] hrgba;
}

static void ReadKernelWeight(const std::string filename, float* _wgt_diff, float* _wgt_spec, int xres, int yres, int patchEle) {
	FILE* fp_diff = fopen((filename + "/kpcn_weight_diff.bin").c_str(), "rb");
	FILE* fp_spec = fopen((filename + "/kpcn_weight_spec.bin").c_str(), "rb");
	fread(_wgt_diff, sizeof(float), yres * xres * patchEle, fp_diff);
	fread(_wgt_spec, sizeof(float), yres * xres * patchEle, fp_spec);
	fclose(fp_diff);
	fclose(fp_spec);

#ifdef DEBUG
	//printf("\nCheck negative weight\n");
	//for (int i = 0; i < yres * xres * patchEle; ++i) {
	//	//if (_wgt_spec[i] < 0.f) {
	//	printf("%f ", _wgt_spec[i]);
	//	//}
	//	if (i % (2 * KERNEL_RADIUS + 1) == 0) {
	//		printf("\n");
	//	}
	//}
#endif // DEBUG
}

static void ComputeRMSE(std::string filename, std::string input_name, float* img, float* ref, int xres, int yres) {
	double relMSE = 0.0;
	const double epsilon = 0.001;

	float* errImg = new float[yres * xres * 3];
	for (int i = 0; i < yres * xres; ++i) {
		double l2err = (img[3 * i + 0] - ref[3 * i + 0]) * (img[3 * i + 0] - ref[3 * i + 0]) +
			(img[3 * i + 1] - ref[3 * i + 1]) * (img[3 * i + 1] - ref[3 * i + 1]) +
			(img[3 * i + 2] - ref[3 * i + 2]) * (img[3 * i + 2] - ref[3 * i + 2]);
		double intensityRef = (ref[3 * i + 0] + ref[3 * i + 1] + ref[3 * i + 2]) / 3.0f;
		double relErr = l2err / (intensityRef * intensityRef + epsilon);
		relMSE += relErr;

		// Visualization
		errImg[3 * i + 0] = (img[3 * i + 0] - ref[3 * i + 0]) * (img[3 * i + 0] - ref[3 * i + 0]) / (float)(intensityRef * intensityRef + epsilon);
		errImg[3 * i + 1] = (img[3 * i + 1] - ref[3 * i + 1]) * (img[3 * i + 1] - ref[3 * i + 1]) / (float)(intensityRef * intensityRef + epsilon);
		errImg[3 * i + 2] = (img[3 * i + 2] - ref[3 * i + 2]) * (img[3 * i + 2] - ref[3 * i + 2]) / (float)(intensityRef * intensityRef + epsilon);
	}
	relMSE /= (double)(yres * xres * 3.0);

	printf("relMSE of %s : %lf\n", input_name.c_str(), relMSE);

	//WriteExr(filename + "_relmse_" + input_name + ".exr", errImg, xres, yres);
}

float box_filter(float(*inputs)[720], int CurrentPos) { // Target Image [0][i][j], [1][i][j], [2][i][j] 를 일반화시켜 받아오기
	
	
	//for (int i = 0; i < 50; i++) //CurrentPos가 모서리가 아닐 때
		
}


class RenderingResult {
public:
	RenderingResult(std::string inputPath, int kernelEle) {
		int xres, yres;
		ReadExrRes(inputPath + "/color.exr", xres, yres);

		inColor = new float[3 * xres * yres];
		inAlbedo = new float[3 * xres * yres];
		inNormal = new float[3 * xres * yres];
		inDepth = new float[1 * xres * yres];
		inFeature = new float[FEATURE_DIMENSION * xres * yres];
		inDiff = new float[3 * xres * yres];
		inSpec = new float[3 * xres * yres];
		outDiff = new float[3 * xres * yres];
		outSpec = new float[3 * xres * yres];
		wgtDiff = new float[kernelEle * xres * yres];
		wgtSpec = new float[kernelEle * xres * yres];

		ReadExr(inputPath + "/color.exr", inColor, 3);
		ReadExr(inputPath + "/albedo.exr", inAlbedo, 3);
		ReadExr(inputPath + "/normal.exr", inNormal, 3);
		ReadExr(inputPath + "/depth.exr", inDepth, 1);
		ReadExr(inputPath + "/diffuse.exr", inDiff, 3);
		ReadExr(inputPath + "/specular.exr", inSpec, 3);
		//ReadExr(inputPath + "_color_mean_OutDiff_360000.exr", outDiff, 3);
		//ReadExr(inputPath + "_color_mean_OutSpec_360000.exr", outSpec, 3);
		ReadKernelWeight(inputPath, wgtDiff, wgtSpec, xres, yres, kernelEle);

		for (int i = 0; i < yres * xres; ++i) {
			//inFeature[FEATURE_DIMENSION * i + 0] = inAlbedo[3 * i + 0];
			//inFeature[FEATURE_DIMENSION * i + 1] = inAlbedo[3 * i + 1];
			//inFeature[FEATURE_DIMENSION * i + 2] = inAlbedo[3 * i + 2];
			//inFeature[FEATURE_DIMENSION * i + 3] = inNormal[3 * i + 0];
			//inFeature[FEATURE_DIMENSION * i + 4] = inNormal[3 * i + 1];
			//inFeature[FEATURE_DIMENSION * i + 5] = inNormal[3 * i + 2];
			//inFeature[FEATURE_DIMENSION * i + 6] = inDepth[i];

			//inFeature[FEATURE_DIMENSION * i + 0] = (inAlbedo[3 * i + 0] + inAlbedo[3 * i + 1] + inAlbedo[3 * i + 2]) / 3.f;
			//inFeature[FEATURE_DIMENSION * i + 1] = (inNormal[3 * i + 0] + inNormal[3 * i + 1] + inNormal[3 * i + 2]) / 3.f;
			//inFeature[FEATURE_DIMENSION * i + 2] = inDepth[i];

			inFeature[FEATURE_DIMENSION * i + 0] = inNormal[3 * i + 0];
			inFeature[FEATURE_DIMENSION * i + 1] = inNormal[3 * i + 1];
			inFeature[FEATURE_DIMENSION * i + 2] = inNormal[3 * i + 2];
		}
	}

	~RenderingResult() {
		delete[] inColor;
		delete[] inAlbedo;
		delete[] inNormal;
		delete[] inDepth;
		delete[] inFeature;
		delete[] inDiff;
		delete[] inSpec;
		delete[] outDiff;
		delete[] outSpec;
		delete[] wgtDiff;
		delete[] wgtSpec;
	}

public:
	float* inColor;
	float* inAlbedo;
	float* inNormal;
	float* inDepth;
	float* inFeature;
	float* inDiff;
	float* inSpec;
	float* outDiff;
	float* outSpec;
	float* wgtDiff;
	float* wgtSpec;
};

class Gradient {
public:
	Gradient(int xres, int yres) {
		color[0] = new float[3 * xres * yres];
		color[1] = new float[3 * xres * yres];
		diff[0] = new float[3 * xres * yres];
		diff[1] = new float[3 * xres * yres];
		spec[0] = new float[3 * xres * yres];
		spec[1] = new float[3 * xres * yres];
	}

	~Gradient() {
		delete[] color[0];
		delete[] color[1];
		delete[] diff[0];
		delete[] diff[1];
		delete[] spec[0];
		delete[] spec[1];
	}

public:
	float* color[2];
	float* diff[2];
	float* spec[2];
};

class BoxFilter { 
public:
	float* ConvertMatrix(float* img, int xres, int yres) { //1280 x 720 resolution -> 1280 x 720 matrix
		int a = 0;
		float TargetImageR[1280][720]; // Each of R G B has 1280 x 720 respectively
		float TargetImageG[1280][720];
		float TargetImageB[1280][720];
		float OutputImg[1280 * 720 * 3]; // equal to input image size

		for (int i = 0; i < 720; i++)
		{
			for (int j = 0; j < 1280; j++)
			{
				TargetImageR[j][0] = img[a];		// 0 3 6 ...
				TargetImageG[j][1] = img[a + 1];	// 1 4 7 ...
				TargetImageB[j][2] = img[a + 2];	// 2 5 8 ...
				a += 3;
			}
			a = 0;
		}

		

		return OutputImg;

	}



}; // BoxFilter