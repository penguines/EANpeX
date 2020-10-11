/**********************************************

Author: penguine

Date: 2020-09-27

Description: Provide functions to assist parsing EAN code.

**********************************************/

#ifndef EANpeX_H
#define EANpeX_H

#include <iostream>
#include <string>
#include <vector>

enum { EAN_THRESHOLD_CONST = 0, EAN_THRESHOLD_AVERAGE = 1 };
enum { EAN_SIGN_START = 1, EAN_SIGN_MIDDLE = 2, EAN_SIGN_END = 3 };

static const char EANLeftA[10][8] = { "0001101","0011001","0010011","0111101","0100011","0110001","0101111","0111011","0110111","0001011" };
static const char EANLeftB[10][8] = { "0100111","0110011","0011011","0100001","0011101","0111001","0000101","0010001","0001001","0010111" };
static const char EANRight[10][8] = { "1110010","1100110","1101100","1000010","1011100","1001110","1010000","1000100","1001000","1110100" };

static int EANCodeAB[10][6] = { {0,0,0,0,0,0},{0,0,1,0,1,1},
								{0,0,1,1,0,1},{0,0,1,1,1,0},
								{0,1,0,0,1,1},{0,1,1,0,0,1},
								{0,1,1,1,0,0},{0,1,0,1,0,1},
								{0,1,0,1,1,0},{0,1,1,0,1,0} };

/*
Parse EAN code to binary string.
	thresholdedData: thresholded data array, only contains 0 and 1.
	n: size of thresholdedData array.
	ean: string used to store output binary EAN result.
	return: 0 -> success ; -1 -> fail
*/
int parseEAN_BIN(int* thresholdedData, int n, std::string& ean);

/*
Parse EAN code to binary string.
	thresholdedData: thresholded data array, only contains 0 and 1.
	n: size of thresholdedData array.
	eanLeft: string used to store the left part of output binary EAN result.
	eanRight: string used to store the right part of output binary EAN result.
	return: 0 -> success ; -1 -> fail
*/
int parseEAN_BIN(int* thresholdedData, int n, std::string& eanLeft, std::string& eanRight);

/*
Parse EAN code to demical string.
	thresholdedData: thresholded data array, only contains 0 and 1.
	n: size of thresholdedData array.
	frontCode: the first number of EAN code, differed by countries or type of goods. E.g. China:6 ; ISBN:9 .
		If given frontCode < 0, frontCode will be tested from 0 to 9 automatically and output the first matched result.
	ean: string used to store output demical EAN result.
	return: 0 -> success ; -1 -> fail
*/
int parseEAN_DEC(int* thresholdedData, int n, int frontCode, std::string& ean);

/*
Get pixel(index) width of next block with given color(black or white)
	index_from: start index where search begins. w
	type: color of next block. 0 -> black ; 1 -> white ;
	direction: search direction. 1 -> from left to right ; -1 -> from right to left.
*/
int getNextBlockWidth(int* thresholdedData, int n, int& index_from, int type, int direction);

/*
Get width of each basic block mode
*/
int matchEANBlocks(int* thresholdedData, int n, float difRate, float& moduleWidth);

/*
Get last index of matched blocks.
	num: number of blocks needed to be matched.
	from: index to start the match.
	fstColor: color of the first block needed to be matched.
	moduleWidth: width(pixel/index) of one basic module.
		Giving moduleWidth = 0 to ignore the first match and get average moduleWidth.
		When moduleWidth != 0, the next blockWidth will be compared with moduleWidth with difRate.
	difRate: maximum difference in pixel(index) allowed in percentage.
		abs(nextBlockWidth - moduleWidth) / moduleWidth <= difRate
	direction: search direction. 1 -> from left to right ; -1 -> from right to left.
*/
int matchEANBlocks(int* thresholdedData,
				   int n,
				   int num,
				   int from,
				   int fstColor,
				   float& moduleWidth,
				   float difRate,
				   int direction);

/*
Find start/middle/end sign of EAN code.
	type: sign needed to find.
		EAN_SIGN_START = 1;
		EAN_SIGN_MIDDLE = 2;
		EAN_SIGN_END = 3 .
	difRate: maximum difference in pixel(index) allowed in percentage.
		abs(nextBlockWidth - moduleWidth) / moduleWidth <= difRate
*/
int findEANSign(int* thresholdedData, int n, int type, float difRate);
/*
Find start/middle/end sign of EAN code and get average basic module width.
	type: sign needed to find.
		EAN_SIGN_START = 1;
		EAN_SIGN_MIDDLE = 2;
		EAN_SIGN_END = 3 .
	difRate: maximum difference in pixel(index) allowed in percentage.
		abs(nextBlockWidth - moduleWidth) / moduleWidth <= difRate
	moduleWidth: variable used to store average basic module width output.
*/
int findEANSign(int* thresholdedData, int n, int type, float difRate, float& moduleWidth);

/*
Find start/middle/end sign of EAN code.
	start/mid/end: variables used to store output sign index.
		If start cannot be found(start = -1), mid&end will also become -1.
	difRate: maximum difference in pixel(index) allowed in percentage.
		abs(nextBlockWidth - moduleWidth) / moduleWidth <= difRate
*/
int findEANSign(int* thresholdedData, int n, int& start, int& mid, int& end, float difRate);

/*
Find start/middle/end sign of EAN code and get average basic module width.
	start/mid/end: variables used to store output sign index.
		If start cannot be found(start = -1), mid&end will also become -1.
	difRate: maximum difference in pixel(index) allowed in percentage.
		abs(nextBlockWidth - moduleWidth) / moduleWidth <= difRate
	moduleWidth: variable used to store average basic module width output.
*/
int findEANSign(int* thresholdedData, int n, int& start, int& mid, int& end, float difRate, float& moduleWidth);

/*
Check whether input EAN code is valid.
	ean: input EAN code array.
	n: size of EAN code array.
	return: 1 -> valid ; 0 -> invalid.
*/
int checkEAN(int* ean, int n);

/*
Calculate check code of EAN barcode.
	eanWithoutChk: input EAN code array without check code.
	n: size of EAN code array.
*/
int calcCheckCode(int* eanWithoutChk, int n);
int calcCheckCode(const char* eanWithoutChk, int n);

/*
Threshold data with certain threshold value.
*/
void threshold(int* data, int n, int thresholdValue);

/*
Threshold data with given threshold type.
	thresholdType: EAN_THRESHOLD_CONST = 0, EAN_THRESHOLD_AVERAGE = 1;
	thresholdValue is available only when thresholdType = EAN_THRESHOLD_CONST.
*/
void threshold(int* data, int n, int thresholdType, int thresholdValue);

/*
Calculate the module number of an EAN Barcode.
	isContainCheck: Whether input EAN Code contains check code.
	genFirstNum: Whether the first number will be generated.
		If input false, the first number will be considered as country code and will not be counted.
		If input true, the first number will be considered as a normal data number and counted.
*/
int calcEANModules(const char* ean, bool isContainCheck = true, bool genFirstNum = false);

/*
Get length of output EAN barcode data.
	ean: EAN Code string.
	moduleWidth: Width of each basic module.
	isContainCheck: Whether input EAN Code contains check code.
	genFirstNum: Whether the first number will be generated.
		If input false, the first number will be considered as country code with no data generated.
		If input true, the first number will be considered as a normal data number and generated.
*/
int genEANData(const char* ean, int moduleWidth, bool isContainCheck = true, bool genFirstNum = false);

/*
Generate EAN barcode light intensity data.
	ean: EAN Code string.
	moduleWidth: Width of each basic module.
	midIndex: Index of middle separator. E.g. Input 6 means the middle separator will be put after the 6th number.
	OutputData: Output data array.
	OutputLength: Length of output data array.
	separatorIndex: Array sized 6 contains left and right index of the start/middle/end separator.
		separatorIndex[0] = left index of start separator;  separatorIndex[1] = right index of start separator.
		separatorIndex[2] = left index of middle separator; separatorIndex[3] = right index of middle separator.
		separatorIndex[4] = left index of end separator;    separatorIndex[5] = right index of end separator.
	isContainCheck: Whether input EAN Code contains check code.
	genFirstNum: Whether the first number will be generated.
		If input false, the first number will be considered as country code with no data generated.
		If input true, the first number will be considered as a normal data number and generated.
*/
int genEANData(const char* ean, int moduleWidth, int midIndex, int* OutputData, int OutputLength, int* separatorIndex = nullptr, bool isContainCheck = true, bool genFirstNum = false);

/*
Get length of EAN-13 Barcode output data.
*/
inline int genEAN13Data(int moduleWidth);

/*
Generate EAN-13 Barcode light intensity data.
*/
int genEAN13Data(const char* ean13, int moduleWidth, int* OutputData, int OutputLength, int* separatorIndex = nullptr, bool isContainCheck = true);

#endif

