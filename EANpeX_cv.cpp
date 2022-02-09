#include "EANpeX_cv.h"

int parseEANMat_DEC(cv::Mat& InputMat, std::string& ean, int EANLength, int frontCode, int thresholdType, float threshold){
	cv::Mat grayMat;

	switch (InputMat.channels()) {
	case 1:
		grayMat = InputMat;
		break;
	case 3:
		cv::cvtColor(InputMat, grayMat, CV_BGR2GRAY);
		break;
	case 4:
		cv::cvtColor(InputMat, grayMat, CV_BGRA2GRAY);
		break;
	default:
		return -1;
	}

	int pixSum = 0;
	int* line = (int*)malloc(grayMat.cols * sizeof(int));
	cv::Mat grayLine;
	for (int i = 0; i < grayMat.rows; i++) {
		pixSum = 0;

		switch (thresholdType) {
		case EAN_THRESHOLD_AVERAGE:
			cv::equalizeHist(grayMat, grayMat);
			grayLine = grayMat.rowRange(i, i + 1);
			for (int j = 0; j < grayLine.cols; j++) {
				line[j] = grayLine.at<uchar>(0, j);
				pixSum += line[j];
			}
			threshold = pixSum / (float)grayLine.cols;
			std::cout << threshold << std::endl;
			for (int j = 0; j < grayLine.cols; j++) {
				line[j] = line[j] >= threshold ? 1 : 0;
			}
			break;

		case EAN_THRESHOLD_CONST:
			grayLine = grayMat.rowRange(i, i + 1);
			for (int j = 0; j < grayLine.cols; j++) {
				line[j] = grayLine.at<uchar>(0, j);
				line[j] = line[j] >= threshold ? 1 : 0;
			}
			break;
		}
		int pse = parseEAN_DEC(line, grayLine.cols, frontCode, ean);
		if (pse == 0 && ean.length() == EANLength)return 0;
	}
	return -1;
}

int findEANArea_Hough(cv::Mat& InputMat, cv::Mat& OutputMat, bool showDetectedLines, bool showDetectedContours){
	CV_Assert(InputMat.channels() == 3);
	cv::Mat imgTmp;
	std::vector<cv::Vec4i> hough_lines;
	
	//图像处理部分
	cv::medianBlur(InputMat, imgTmp, 5);
	cv::cvtColor(imgTmp, imgTmp, CV_BGR2GRAY);
	cv::Canny(imgTmp, imgTmp, 50, 150);
	cv::HoughLinesP(imgTmp, hough_lines, 1, CV_PI / 180., 100, 100, 8);

	//霍夫变化结果处理
	//找到数量最多倾角值相的同直线对应的角度
	float angle;
	int angleCnt[180] = { 0 };
	int arrayIndex;
	std::vector<std::vector<int>> angleIndex(180);
	for (int i = 0; i < hough_lines.size(); i++) {
		angle = atan((hough_lines[i][1] - hough_lines[i][3]) / (float)(hough_lines[i][0] - hough_lines[i][2])) * 180. / CV_PI;
		arrayIndex = angle + 89;
		angleCnt[arrayIndex]++;
		angleIndex[arrayIndex].push_back(i);
	}
	int maxNumAngle = 0;
	int maxNum = 0;
	for (int i = 0; i < 180; i++) {
		if (angleCnt[i] > maxNum) {
			maxNum = angleCnt[i];
			maxNumAngle = i;
		}
	}
	//考虑到误差，将倾角为所得角度正负1deg内的直线都计入判断范围
	int maxA1 = angleAdd(maxNumAngle, 1), maxM1 = angleAdd(maxNumAngle, -1);
	for (int i = 0; i < angleIndex[maxA1].size(); i++) {
		angleIndex[maxNumAngle].push_back(angleIndex[maxA1][i]);
	}
	for (int i = 0; i < angleIndex[maxM1].size(); i++) {
		angleIndex[maxNumAngle].push_back(angleIndex[maxM1][i]);
	}

	//对指定倾角的直线聚集程度进行判断
	//dis: 坐标系旋转后，同组直线之间最远距离(pix)
	int dis = 70;
	float x_max = 0, x_min = 0;
	float x01, x02, y01, y02;
	float x1, x2;
	float y_max = 0, y_min = 0;
	float y1, y2;
	float sina, cosa;
	//储存直线组
	std::vector<std::vector<int>> lineGroups;
	std::vector<int> group;
	//储存边界点
	std::vector<std::vector<cv::Point2f>> borderPt;
	std::vector<cv::Point2f> pts;
	//计算坐标系旋转参数
	if (maxNumAngle <= 90) {
		sina = sin(maxNumAngle * DEG2RAD);
		cosa = cos(maxNumAngle * DEG2RAD);
	}
	else {
		sina = sin((maxNumAngle - 180) * DEG2RAD);
		cosa = cos((maxNumAngle - 180) * DEG2RAD);
	}

	while (angleIndex[maxNumAngle].size() > 0) {
		//坐标系旋转maxNumAngle，使得Y轴与直线平行
		//以下边界点的计算均在旋转后的坐标系中进行
		x01 = hough_lines[angleIndex[maxNumAngle][0]][0];
		x02 = hough_lines[angleIndex[maxNumAngle][0]][2];
		y01 = hough_lines[angleIndex[maxNumAngle][0]][1];
		y02 = hough_lines[angleIndex[maxNumAngle][0]][3];
		x1 = x01 * cosa + y01 * sina;
		y1 = x01 * (-sina) + y01 * cosa;
		x2 = x02 * cosa + y02 * sina;
		y2 = x02 * (-sina) + y02 * cosa;
		//保证x1 > x2; y1 > y2
		if (x2 > x1)std::swap(x1, x2);
		if (y2 > y1)std::swap(y1, y2);
		x_max = x1;
		x_min = x2;
		pts.push_back(cv::Point2f(x2, y2));
		pts.push_back(cv::Point2f(x1, y1));
		y_max = y1;
		y_min = y2;
		pts.push_back(cv::Point2f(x2, y2));
		pts.push_back(cv::Point2f(x1, y1));

		std::vector<int>::iterator it;
		std::vector<double> yMid;
		std::vector<bool> isInGroup;
		std::vector<cv::Vec4f> transCord;
		double ySum = 0, yAve = 0, ySigma = 0;
		int yCount = 0;

		//计算线段中点纵坐标平均值及方差，根据3sigma法则去除过长的线段
		for (it = angleIndex[maxNumAngle].begin(); it != angleIndex[maxNumAngle].end(); it++) {
			x01 = hough_lines[*it][0];
			x02 = hough_lines[*it][2];
			y01 = hough_lines[*it][1];
			y02 = hough_lines[*it][3];
			x1 = x01 * cosa + y01 * sina;
			y1 = x01 * (-sina) + y01 * cosa;
			x2 = x02 * cosa + y02 * sina;
			y2 = x02 * (-sina) + y02 * cosa;
			if (x2 > x1)std::swap(x1, x2);
			if (y2 > y1)std::swap(y1, y2);
			if ((x1 <= dis + x_max && x1 >= x_min - dis) || (x2 <= dis + x_max && x2 >= x_min - dis)) {
				cv::Vec4f cord(x1, y1, x2, y2);
				transCord.push_back(cord);
				isInGroup.push_back(true);
				yMid.push_back((y1 + y2) / 2.);
				ySum += yMid[yCount];
				yCount++;
				if (x1 > x_max) {
					x_max = x1;
					pts[1] = cv::Point2f(x1, y1);
				}
				if (x2 < x_min) {
					x_min = x2;
					pts[0] = cv::Point2f(x2, y2);
				}
			}
			else {
				isInGroup.push_back(false);
			}
		}
		yAve = ySum / yCount;
		ySum = 0;
		for (auto it = yMid.begin(); it != yMid.end(); it++) {
			ySum += ((*it) - yAve) * ((*it) - yAve);
		}
		ySigma = sqrt(ySum / (yCount - 1));
		yCount = 0;
		int j = 0;
		for (it = angleIndex[maxNumAngle].begin(); it != angleIndex[maxNumAngle].end();) {
			if (isInGroup[j] == true) {
				if (fabs(yMid[yCount] - yAve) <= 3 * ySigma) {
					x1 = transCord[yCount][0];
					y1 = transCord[yCount][1];
					x2 = transCord[yCount][2];
					y2 = transCord[yCount][3];
					if (y1 > y_max) {
						y_max = y1;
						pts[3] = cv::Point2f(x1, y1);
					}
					if (y2 < y_min) {
						y_min = y2;
						pts[2] = cv::Point2f(x2, y2);
					}
					if (x1 == x_max) {
						pts[1] = cv::Point2f(x1, y1);
					}
					if (x2 == x_min) {
						pts[0] = cv::Point2f(x2, y2);
					}
					group.push_back(*it);
					it = angleIndex[maxNumAngle].erase(it);
				}
				else {
					it = angleIndex[maxNumAngle].erase(it);
				}
				yCount++;
			}
			else {
				it++;
			}
			j++;
		}

		borderPt.push_back(pts);
		lineGroups.push_back(group);
		pts.clear();
		group.clear();
	}

	//查找包含直线数最多的直线组，认为其就是条形码区域
	int lineMax = 0, indexLM = 0;
	if (lineGroups.size() > 0) {
		lineMax = lineGroups[0].size();
		for (int i = 1; i < lineGroups.size(); i++) {
			if (lineGroups[i].size() > lineMax) {
				lineMax = lineGroups[i].size();
				indexLM = i;
			}
		}
	}
	else return -1;

	//获取直线组边界点，划定区域，反变换回到原图片坐标系下
	if (borderPt.size() > 0) {
		int extendX = 80, extendY = 20;
		cv::Point2f dstPoints[4], srcPoints[4];
		dstPoints[0] = cv::Point(0, 0);
		dstPoints[1] = cv::Point(OutputMat.cols, 0);
		dstPoints[2] = cv::Point(OutputMat.cols, OutputMat.rows);
		dstPoints[3] = cv::Point(0, OutputMat.rows);
		srcPoints[0] = cv::Point2f(borderPt[indexLM][0].x - extendX, borderPt[indexLM][2].y - extendY);
		srcPoints[1] = cv::Point2f(borderPt[indexLM][1].x + extendX, borderPt[indexLM][2].y - extendY);
		srcPoints[2] = cv::Point2f(borderPt[indexLM][1].x + extendX, borderPt[indexLM][3].y + extendY);
		srcPoints[3] = cv::Point2f(borderPt[indexLM][0].x - extendX, borderPt[indexLM][3].y + extendY);
		//边界点坐标反变换回到原坐标系下
		for (int i = 0; i < 4; i++) {
			x1 = srcPoints[i].x;
			y1 = srcPoints[i].y;
			srcPoints[i].x = x1 * cosa - y1 * sina;
			srcPoints[i].y = x1 * sina + y1 * cosa;
		}

		//获取透视变换矩阵，将原图像中条形码部分对应到输出图像
		cv::Mat transMat = cv::getPerspectiveTransform(srcPoints, dstPoints);
		cv::warpPerspective(InputMat, OutputMat, transMat, OutputMat.size());

		//***********************展示线条**************************
		if (lineGroups.size() != 0) {
			if (showDetectedLines == true) {
				for (int i = 0; i < lineMax; i++) {
					int lineIndex = lineGroups[indexLM][i];
					cv::line(InputMat, cv::Point(hough_lines[lineIndex][0], hough_lines[lineIndex][1]), cv::Point(hough_lines[lineIndex][2], hough_lines[lineIndex][3]), cv::Scalar(0, 0, 255), 2, CV_AA);
				}
			}
		}
		//********************************************************

		//***********************展示边界**************************
		if (showDetectedContours == true) {
			std::vector<cv::Point> extendedRect;
			std::vector<std::vector<cv::Point>> houghRect;
			for (int i = 0; i < 4; i++)extendedRect.push_back(cv::Point(srcPoints[i].x, srcPoints[i].y));
			houghRect.push_back(extendedRect);
			cv::drawContours(InputMat, houghRect, 0, cv::Scalar(0, 255, 0), 2, CV_AA);
		}
		//********************************************************
		return 0;
	}
	else return -1;
}

int findEANArea_Contours(cv::Mat& InputMat, cv::Mat& OutputMat, bool showDetectedContours){
	CV_Assert(InputMat.channels() == 3);
	cv::Mat imgTmp;

	//图像处理部分
	cv::medianBlur(InputMat, imgTmp, 5);
	cv::cvtColor(imgTmp, imgTmp, CV_BGR2GRAY);
	cv::Canny(imgTmp, imgTmp, 50, 150);

	//轮廓检测
	std::vector<cv::Vec4i> hierarchy;
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(imgTmp, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);

	//多边形逼近轮廓，寻找面积最大的四边形轮廓
	if (contours.size() > 0) {
		int maxIndex = -1;
		double maxArea = 0, tmpArea;
		std::vector<std::vector<cv::Point>> polyContours(contours.size());
		for (int i = 0; i < contours.size(); i++) {
			cv::approxPolyDP(contours[i], polyContours[i], 10, true);
			if ((tmpArea = cv::contourArea(polyContours[i])) > maxArea&& polyContours[i].size() >= 4 && polyContours[i].size() <= 6) {
				maxArea = tmpArea;
				maxIndex = i;
			}
		}

		if (maxIndex >= 0) {
			cv::Point2f dstPoints[4], srcPoints[4];
			dstPoints[0] = cv::Point(0, 0);
			dstPoints[1] = cv::Point(OutputMat.cols, 0);
			dstPoints[2] = cv::Point(OutputMat.cols, OutputMat.rows);
			dstPoints[3] = cv::Point(0, OutputMat.rows);

			bool sorted = 0;

			while (!sorted) {
				for (int i = 1; i < 4; i++) {
					sorted = 1;
					if (polyContours[maxIndex][i - 1].x > polyContours[maxIndex][i].x) {
						swap(polyContours[maxIndex][i - 1], polyContours[maxIndex][i]);
						sorted = 0;
					}
				}
			}

			if (polyContours[maxIndex][0].y < polyContours[maxIndex][1].y) {
				srcPoints[0] = polyContours[maxIndex][0];
				srcPoints[3] = polyContours[maxIndex][1];
			}
			else {
				srcPoints[0] = polyContours[maxIndex][1];
				srcPoints[3] = polyContours[maxIndex][0];
			}

			if (polyContours[maxIndex][2].y < polyContours[maxIndex][3].y) {
				srcPoints[1] = polyContours[maxIndex][2];
				srcPoints[2] = polyContours[maxIndex][3];
			}
			else {
				srcPoints[1] = polyContours[maxIndex][3];
				srcPoints[2] = polyContours[maxIndex][2];
			}

			cv::Mat transMat = cv::getPerspectiveTransform(srcPoints, dstPoints);
			cv::warpPerspective(InputMat, OutputMat, transMat, OutputMat.size());

			//***********************展示边界**************************
			if (showDetectedContours == true) {
				cv::drawContours(InputMat, polyContours, maxIndex, cv::Scalar(0, 255, 0), 1, CV_AA, hierarchy);
			}
			//********************************************************

			return 0;
		}
		else return -1;
	}
	else return -1;
}

void rebuildEAN(cv::Mat& OutputArray, int OutputWidth, int OutputHeight, int* thresholdedData, int n){
	OutputArray = cv::Mat(OutputHeight, OutputWidth, CV_8UC1, cv::Scalar(255));
	float k = OutputWidth / (float)n;
	float x;
	for (int i = 0; i < n; i++) {
		x = i * k;
		cv::line(OutputArray, cv::Point2f(x, 0), cv::Point2f(x, OutputHeight), cv::Scalar(255 * thresholdedData[i]), 2, CV_AA);
	}
}

void genEANBarcode(cv::Mat& OutputArray, int width, int height, const char* ean, int midIndex, bool isContainCheck, bool genFirstNum, bool showNumbers){
	CV_Assert(width > 0 && height > 0);
	OutputArray = cv::Mat(height, width, CV_8UC1, cv::Scalar(255));
	
	int eanLength = strlen(ean);
	//计算分隔符、普通数据条像素长度以及数字高度
	const float separatorLengthRatio = 0.93, dataBarLengthRatio = 0.85;
	const double fontWidthRatio = 0.95;
	const double fontScaleRatio = 3. / 64.;	//using cv::FONT_HERSHEY_COMPLEX or cv::FONT_HERSHEY_SIMPLEX
	const double fontThickRatio = 1.7;
	const double fontTopDistRatio = 0.02;
	int separatorLength = separatorLengthRatio * height;
	int dataBarLength = dataBarLengthRatio * height;
	//字体大小及顶距
	int font = cv::FONT_HERSHEY_SIMPLEX;
	int fontTopDistance = fontTopDistRatio * height;
	if (fontTopDistance == 0)fontTopDistance = 1;
	int fontHeight = height - fontTopDistance - dataBarLength, fontWidth;


	//计算模组宽度
	int moduleCnt = calcEANModules(ean, isContainCheck, genFirstNum);
	int moduleWidth = width / moduleCnt;
	//x * 7 = x * 8 - x = (x << 3) - x;
	int numWidth = (moduleWidth << 3) - moduleWidth;
	fontWidth = numWidth * fontWidthRatio;
	//获取EAN数据数组长
	int eanDataLength = genEANData(ean, moduleWidth, isContainCheck, genFirstNum);
	//创建EAN数据储存数组及分隔符位置数组
	int* eanData = (int*)malloc(eanDataLength * sizeof(int));
	int eanSeparator[7] = { 0 };
	//生成EAN数据
	genEANData(ean, moduleWidth, midIndex, eanData, eanDataLength, eanSeparator, isContainCheck, genFirstNum);

	//绘图
	int separatorTmp = eanSeparator[0];
	for (int i = 0; i < 5;) {
		i++;
		for (int pix = separatorTmp; pix < eanSeparator[i]; pix++) {
			if (eanData[pix] == 0) {
				cv::line(OutputArray, cv::Point(pix, 0), cv::Point(pix, separatorLength - 1), cv::Scalar(0), 1, CV_AA);
			}
		}
		separatorTmp = eanSeparator[i + 1];
		for (int pix = eanSeparator[i]; pix < separatorTmp; pix++) {
			if (eanData[pix] == 0) {
				cv::line(OutputArray, cv::Point(pix, 0), cv::Point(pix, dataBarLength - 1), cv::Scalar(0), 1, CV_AA);
			}
		}
		i++;
	}
	
	//绘制数字
	if (showNumbers) {
		//计算fontScale
		double fontScale = fontScaleRatio * fontHeight;
		int fontThick = fontScale * fontThickRatio;
		if (fontThick < 1)fontThick = 1;
		//修正fontScale
		fontScale -= fontScaleRatio * ((fontThick - 1) >> 1);
		//获取数字大小
		int baseline;
		cv::Size fontSize = cv::getTextSize("0", font, fontScale, fontThick, &baseline);
		//判断宽度
		if (fontSize.width > fontWidth) {
			fontScale *= (fontWidth / (double)fontSize.width);
			//重新计算字体线宽及修正fontScale
			fontThick = fontScale * fontThickRatio;
			if (fontThick < 1)fontThick = 1;
			fontScale -= fontScaleRatio * ((fontThick - 1) >> 1);
			//重新获取数字大小
			fontSize = cv::getTextSize("0", font, fontScale, fontThick, &baseline);
		}

		//绘制数字
		int index = 0;
		int textBottom = fontSize.height + fontTopDistance + dataBarLength;
		int textLeft = (numWidth - fontSize.width) >> 1;
		int x = 0;
		char text[2];
		text[1] = '\0';
		if (genFirstNum == false) {
			x = eanSeparator[0] - numWidth;
			text[0] = ean[0];
			cv::putText(OutputArray, text, cv::Point(x + textLeft, textBottom), font, fontScale, cv::Scalar(0), fontThick, CV_AA);
			index++;
		}
		x = eanSeparator[1];
		for (; index <= midIndex; index++) {
			text[0] = ean[index];
			cv::putText(OutputArray, text, cv::Point(x + textLeft, textBottom), font, fontScale, cv::Scalar(0), fontThick, CV_AA);
			x += numWidth;
		}
		x = eanSeparator[3];
		for (; index < eanLength; index++) {
			text[0] = ean[index];
			cv::putText(OutputArray, text, cv::Point(x + textLeft, textBottom), font, fontScale, cv::Scalar(0), fontThick, CV_AA);
			x += numWidth;
		}
		if (isContainCheck == false) {
			int checkCode = calcCheckCode(ean, eanLength);
			text[0] = checkCode + '0';
			cv::putText(OutputArray, text, cv::Point(x + textLeft, textBottom), font, fontScale, cv::Scalar(0), fontThick, CV_AA);
		}
	}
}

void genEAN13Barcode(cv::Mat& OutputArray, int width, int height, const char* ean13, bool isContainCheck, bool showNumbers){
	int eanLength = strlen(ean13);
	CV_Assert((isContainCheck == false && eanLength == 12) || (isContainCheck == true && eanLength == 13));
	genEANBarcode(OutputArray, width, height, ean13, 6, isContainCheck, false, showNumbers);
}

int angleAdd(int angle1, int angle2) {
	int result = angle1 + angle2;
	while (result >= 180)result -= 180;
	while (result < 0)result += 180;
	return result;
	return 0;
}

void cvContrastBright(cv::Mat& srcArray, cv::Mat& dstArray, int contrastValue, int brightValue) {
	cv::Mat dst = cv::Mat::zeros(srcArray.size(), srcArray.type());
	if (srcArray.channels() == 1) {
		for (int y = 0; y < srcArray.rows; y++) {
			for (int x = 0; x < srcArray.cols; x++) {
					dst.at<uchar>(y, x) = cv::saturate_cast<uchar>((contrastValue * 0.01) * (srcArray.at<uchar>(y, x)) + brightValue);
			}
		}
	}
	else if (srcArray.channels() == 3) {
		for (int y = 0; y < srcArray.rows; y++) {
			for (int x = 0; x < srcArray.cols; x++) {
				for (int i = 0; i < 3; i++) {
					dst.at<cv::Vec3b>(y, x)[i] = cv::saturate_cast<uchar>((contrastValue * 0.01) * (srcArray.at<cv::Vec3b>(y, x)[i]) + brightValue);
				}
			}
		}
	}
	dstArray = dst.clone();
	return;
}