// filters.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>

#include "Image.h"

int main()
{

	int xres, yres;
	float* rgb;
	std::string colorPath = "C:\\Users\\juni\\Desktop\\project\\curly-hair_128\\color.exr";

	ReadExrRes(colorPath, xres, yres); // x res : 1280 , y res : 720
	ReadExr(colorPath, rgb);

	float* img = new float[yres * xres * 3];
	

	//std::cout << "X resolution : " << xres << std::endl;
	//std::cout << "Y resolution : " << yres << std::endl;
	BoxFilter boxfilter;
	boxfilter.ConvertMatrix(img, xres, yres);

	return 0;

}


