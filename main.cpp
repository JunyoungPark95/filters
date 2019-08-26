#include "Image.h"
#include "filters.h"

#include "filterCUDA.h"

using namespace std;

int main(int argc, char *argv[]) {
	std::string filterType;
	std::string inputColorPath = "C:\\Users\\junyoung\\Desktop\\project\\denoising\\curly-hair_128\\color.exr";
	std::string writePath = "C:\\Users\\junyoung\\Desktop\\project\\denoising\\curly-hair_128\\";
	int xres, yres;
	float timeEval[2] = { 0.f, };

	ReadExrRes(inputColorPath, xres, yres); // x res : 1280 , y res : 720
	float *inputColor = new float[yres * xres * 3];
	ReadExr(inputColorPath, inputColor);

	//// CPU implementation
	Filter filter;
	float* estColor = filter.BoxFiltering(inputColor, xres, yres, timeEval);
	filterType = "Box_CPU";
	WriteExr(writePath + filterType + ".exr", estColor, xres, yres);

	// CUDA implementation
	float *estColor2 = new float[yres * xres * 3];
	BoxFiltering(estColor2, inputColor, xres, yres, timeEval);
	filterType = "Box_CUDA";
	WriteExr(writePath + filterType + ".exr", estColor2, xres, yres);

	std::cout << "CPU based filter time : " << timeEval[0] << std::endl;
	std::cout << "GPU based filter time : " << timeEval[1] << std::endl;

	printf("\nDone");
	return 0;
}