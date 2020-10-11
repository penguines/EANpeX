#include <iostream>
#include <opencv2/opencv.hpp>
#include "EANpeX_cv.h"

int main() {
	std::string eanOut;
	cv::Mat eanImg;

	//Generate EAN-13 Barcode image with complete EAN Code.
	const char eanWithCheckCode[] = "9787532736553";
	genEAN13Barcode(eanImg, 500, 300, eanWithCheckCode);
	cv::imshow("EAN-13 Barcode-1", eanImg);
	//Parse generated EAN-13 Barcode image.
	if (parseEANMat_DEC(eanImg, eanOut, 13, -1) == 0)std::cout << "Parse result of complete EAN Code: " << eanOut << std::endl;
	

	//Generate EAN-13 Barcode image with EAN Code without check code, using automatic inferred check code.
	const char eanNoCheckCode[] = "692225545142";
	genEAN13Barcode(eanImg, 500, 300, eanNoCheckCode, false);
	cv::imshow("EAN-13 Barcode-2", eanImg);
	//Parse generated EAN-13 Barcode image.
	if (parseEANMat_DEC(eanImg, eanOut, 13, -1) == 0)std::cout << "Parse result of EAN Code without check code: " << eanOut << std::endl;
	
	cv::waitKey(0);
	return 0;
}