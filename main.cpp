#include "Image.h"
#include "filters.h"
using namespace std;

int main(int argc, char *argv[]) {

	int xres, yres;

	std::string inputColorPath = "C:\\Users\\junyoung\\Desktop\\project\\denoising\\curly-hair_128\\color.exr";
	
	ReadExrRes(inputColorPath, xres, yres); // x res : 1280 , y res : 720
	float *inputColor = new float[yres * xres * 3];
	ReadExr(inputColorPath, inputColor);

	Filter filter;
	float* estColor = filter.BoxFiltering(inputColor, xres, yres);
	std::string writePath = "C:\\Users\\junyoung\\Desktop\\project\\denoising\\curly-hair_128\\filtered_color.exr";
	WriteExr(writePath, estColor, xres, yres);
	return 0;
}