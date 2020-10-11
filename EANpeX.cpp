#include "EANpeX.h"

int parseEAN_BIN(int* thresholdedData, int n, std::string& ean) {
	std::string eanLeft, eanRight;
	int result = parseEAN_BIN(thresholdedData, n, eanLeft, eanRight);
	ean = eanLeft + eanRight;
	return result;
}

//return 0 -> success ; return -1 -> fail
int parseEAN_BIN(int* thresholdedData, int n, std::string& eanLeft, std::string& eanRight) {
	int index = 0, index_out = 0;
	float moduleWidth = 0;
	int modulePcs, nextBlockWidth;
	int wbflag = 1, start, mid_r, mid_l, end;
	char out[120] = { 0 };
	findEANSign(thresholdedData, n, start, mid_r, end, 0.5, moduleWidth);
	if (start != -1 && mid_r != -1 && end != -1) {
		index = start;
		mid_l = matchEANBlocks(thresholdedData, n, 5, mid_r, 1, moduleWidth, 0.5, -1);
		while (index < mid_l) {
			nextBlockWidth = getNextBlockWidth(thresholdedData, n, index, wbflag, 1);
			modulePcs = (nextBlockWidth / moduleWidth) + 0.5;
			//每块均由1-4个模块构成
			if (modulePcs < 1 || modulePcs > 4)return -1;
			for (int i = 0; i < modulePcs; i++) {
				out[index_out] = '1' - wbflag;
				index_out++;
			}
			wbflag = wbflag ^ 1;
		}
		out[index_out] = '\0';
		eanLeft = out;
		index_out = 0;
		index = mid_r;
		wbflag = 0;
		while (index < end) {
			nextBlockWidth = getNextBlockWidth(thresholdedData, n, index, wbflag, 1);
			modulePcs = (nextBlockWidth / moduleWidth) + 0.5;
			//每块均由1-4个模块构成
			if (modulePcs < 1 || modulePcs > 4)return -1;
			for (int i = 0; i < modulePcs; i++) {
				out[index_out] = '1' - wbflag;
				index_out++;
			}
			wbflag = wbflag ^ 1;
		}
		out[index_out] = '\0';
		eanRight = out;
		return 0;
	}
	else return -1;
}

int parseEAN_DEC(int* thresholdedData, int n, int frontCode, std::string& ean) {
	std::string bineanL, bineanR;
	int index = 0, index_out = 1;
	int ub;
	int findflag = 0;
	std::string tmp;
	char out[15] = { 0 };
	int num[15] = { 0 };
	if (parseEAN_BIN(thresholdedData, n, bineanL, bineanR) == 0) {
		if (frontCode >= 0) {
			out[0] = '0' + frontCode;
			num[0] = frontCode;
		}
		if (bineanL.length() % 7 > 0)return -1;
		if (bineanR.length() % 7 > 0)return -1;

		ub = bineanL.length() - 7;
		while (index <= ub) {
			findflag = 0;
			tmp = bineanL.substr(index, 7);
			for (int i = 0; i < 10; i++) {
				if (strcmp(tmp.c_str(), EANLeftA[i]) == 0 || strcmp(tmp.c_str(), EANLeftB[i]) == 0) {
					out[index_out] = i + '0';
					num[index_out] = i;
					index_out++;
					findflag = 1;
					break;
				}
			}
			if (findflag == 0) {
				return -21;
			}
			index += 7;
		}

		ub = bineanR.length() - 7;
		index = 0;
		while (index <= ub) {
			findflag = 0;
			tmp = bineanR.substr(index, 7);
			for (int i = 0; i < 10; i++) {
				if (strcmp(tmp.c_str(), EANRight[i]) == 0) {
					out[index_out] = i + '0';
					num[index_out] = i;
					index_out++;
					findflag = 1;
					break;
				}
			}
			if (findflag == 0) {
				return -22;
			}
			index += 7;
		}
		
		if (frontCode >= 0) {
			if (checkEAN(num, index_out))return -3;
		}
		else {
			bool isMatched = false;
			for (int i = 0; i < 10; i++) {
				num[0] = i;
				if (checkEAN(num, index_out)) {
					out[0] = '0' + i;
					isMatched = true;
					break;
				}
			}
			if (isMatched == false)return -3;
		}
		out[index_out] = '\0';
		ean = out;
		return 0;
	}
	else {
		return -1;
	}

}

int getNextBlockWidth(int* thresholdedData, int n, int& index_from, int type, int direction) {
	int begin;
	while (thresholdedData[index_from] != type) {
		index_from += direction;
		if (index_from >= n || index_from < 0)return 0; // 0 : Fail to find
	}
	begin = index_from;
	while (thresholdedData[index_from] == type) {
		index_from += direction;
		if (index_from >= n || index_from < 0)return 0; // 0 : Fail to find
	}
	return abs(index_from - begin);
}

int matchEANBlocks(int* thresholdedData, int n, float difRate, float& moduleWidth)
{
	moduleWidth = 0;
	return matchEANBlocks(thresholdedData, n, 3, 0, 0, moduleWidth, difRate, 1);
}

int matchEANBlocks(int* thresholdedData, int n, int num, int from, int fstColor, float& moduleWidth, float difRate, int direction)
{
	if (from < 0 || from >= n)return -1;
	int index = from;
	int blockWidth, nextBlockWidth;
	int color, flag = 0;
	float matchWidth, widthSum = 0;
	while (index < n && index >= 0) {
		flag = 0;
		color = fstColor;
		blockWidth = getNextBlockWidth(thresholdedData, n, index, color, direction);
		if (moduleWidth == 0)matchWidth = blockWidth;
		else {
			matchWidth = moduleWidth;
			if ((abs(blockWidth - matchWidth) / matchWidth) > difRate)continue;
		}
		widthSum = matchWidth;
		if (blockWidth > 0) {
			for (int i = 1; i < num; i++) {
				color = color ^ 1;
				nextBlockWidth = getNextBlockWidth(thresholdedData, n, index, color, direction);
				if ((abs(nextBlockWidth - matchWidth) / matchWidth) > difRate) {
					flag = 1;
					break;
				}
				else widthSum += nextBlockWidth;
			}
			if (!flag) {
				if (moduleWidth == 0)moduleWidth = widthSum / (float)num;
				return index;
			}
		}
	}
	return -1;
}

int findEANSign(int* thresholdedData, int n, int type, float difRate) {
	float mw = 0;
	return findEANSign(thresholdedData, n, type, difRate, mw);
}

int findEANSign(int* thresholdedData, int n, int type, float difRate, float& moduleWidth) {
	int indexTmp = -1;
	float mw = 0;
	switch (type) {
	case EAN_SIGN_START:
		indexTmp = matchEANBlocks(thresholdedData, n, difRate, mw);
		break;
	case EAN_SIGN_MIDDLE:
		indexTmp = matchEANBlocks(thresholdedData, n, difRate, mw);
		indexTmp = matchEANBlocks(thresholdedData, n, 5, indexTmp, 1, mw, difRate, 1);
		break;
	case EAN_SIGN_END:
		indexTmp = matchEANBlocks(thresholdedData, n, 3, n - 1, 0, mw, difRate, -1);
		break;
	}
	moduleWidth = mw;
	return indexTmp;
}

int findEANSign(int* thresholdedData, int n, int& start, int& mid, int& end, float difRate) {
	float tmp = 0;
	return findEANSign(thresholdedData, n, start, mid, end, difRate, tmp);
}

int findEANSign(int* thresholdedData, int n, int& start, int& mid, int& end, float difRate, float& moduleWidth) {
	float mw = 0;
	start = matchEANBlocks(thresholdedData, n, difRate, mw);
	moduleWidth = mw;
	mid = matchEANBlocks(thresholdedData, n, 5, start, 1, mw, difRate, 1);
	end = matchEANBlocks(thresholdedData, n, 3, n - 1, 0, mw, difRate, -1);
	return 0;
}

int checkEAN(int* ean, int n){
	if (ean[n - 1] != calcCheckCode(ean, n - 1))return 0;
	else return 1;
}

int calcCheckCode(int* eanWithoutChk, int n){
	int sumEven = 0, sumOdd = 0;
	for (int i = 1; i < n; i += 2) {
		sumOdd += eanWithoutChk[i - 1];
		sumEven += eanWithoutChk[i];
	}
	return (10 - ((sumEven * 3 + sumOdd) % 10));
}

int calcCheckCode(const char* eanWithoutChk, int n){
	int sumEven = 0, sumOdd = 0;
	for (int i = 1; i < n; i += 2) {
		sumOdd += eanWithoutChk[i - 1] - '0';
		sumEven += eanWithoutChk[i] - '0';
	}
	return (10 - ((sumEven * 3 + sumOdd) % 10));
	return 0;
}

void threshold(int* data, int n, int thresholdValue){
	for (int i = 0; i < n; i++) {
		if (data[i] > thresholdValue)data[i] = 1;
		else data[i] = 0;
	}
}

void threshold(int* data, int n, int thresholdType, int thresholdValue){
	int sum = 0;
	switch (thresholdType) {
	case EAN_THRESHOLD_AVERAGE:
		for (int i = 0; i < n; i++) {
			sum += data[i];
		}
		thresholdValue = sum / n;
		break;
	case EAN_THRESHOLD_CONST:
		break;
	default:
		thresholdValue = 127;
	}
	threshold(data, n, thresholdValue);
}

int calcEANModules(const char* ean, bool isContainCheck, bool genFirstNum){
	int i = 0;
	int moduleCnt = 0;
	while (ean[i] >= '0' && ean[i] <= '9')i++;

	if (i > 0) {
		//输入中是否包含校验位
		if (isContainCheck == false)i++;
		//是否生成第一位的条码
		if (genFirstNum == false)i--;
	}
	//有效数字
	moduleCnt = i * 7;
	//起始符,中间分隔符,终止符占位
	moduleCnt += 11;
	//左右两侧空白区占位
	moduleCnt += 18;
	return moduleCnt;
}

int genEANData(const char* ean, int moduleWidth, bool isContainCheck, bool genFirstNum){
	return calcEANModules(ean, isContainCheck, genFirstNum) * moduleWidth;
}
	
int genEANData(const char* ean, int moduleWidth, int midIndex, int* OutputData, int OutputLength, int* separatorIndex, bool isContainCheck, bool genFirstNum){
	int checkCode, nationCode = ean[0] - '0';
	int index = 0, pix = 0;
	int eanLength = strlen(ean), lenTmp;
	int separator[3][2];

	char EANLeft[2][10][8];
	for (int i = 0; i < 10; i++) {
		strcpy(EANLeft[0][i], EANLeftA[i]);
		strcpy(EANLeft[1][i], EANLeftB[i]);
	}

	if (isContainCheck == false)checkCode = calcCheckCode(ean, eanLength);
	if (genFirstNum == false)index = 1;

	//左侧空白区
	lenTmp = 11 * moduleWidth;
	for (pix = 0; pix < lenTmp; pix++) {
		OutputData[pix] = 1;
	}
	//起始符
	int sign = 0;
	separator[0][0] = pix;
	for (int i = 0; i < 3; i++) {
		lenTmp += moduleWidth;
		for (; pix < lenTmp; pix++) {
			OutputData[pix] = sign;
		}
		sign = sign ^ 1;
	}
	separator[0][1] = pix;

	//Left Part
	int EANLeftAB, indexEAN, pixVal;
	for (; index <= midIndex; index++) {
		if (index < 7)EANLeftAB = EANCodeAB[nationCode][index - 1];
		else EANLeftAB = 0;
		indexEAN = ean[index] - '0';
		//7位二进制
		for (int k = 0; k < 7; k++) {
			pixVal = '1' - EANLeft[EANLeftAB][indexEAN][k];
			for (int i = 0; i < moduleWidth; i++) {
				OutputData[pix] = pixVal;
				pix++;
			}
		}
	}

	//中间分隔符
	lenTmp = pix;
	sign = 1;
	separator[1][0] = pix;
	for (int i = 0; i < 5; i++) {
		lenTmp += moduleWidth;
		for (; pix < lenTmp; pix++) {
			OutputData[pix] = sign;
		}
		sign = sign ^ 1;
	}
	separator[1][1] = pix;

	//Right Part
	for (; index < eanLength; index++) {
		indexEAN = ean[index] - '0';
		for (int k = 0; k < 7; k++) {
			pixVal = '1' - EANRight[indexEAN][k];
			for (int i = 0; i < moduleWidth; i++) {
				OutputData[pix] = pixVal;
				pix++;
			}
		}
	}
	if (isContainCheck == false) {
		for (int k = 0; k < 7; k++) {
			pixVal = '1' - EANRight[checkCode][k];
			for (int i = 0; i < moduleWidth; i++) {
				OutputData[pix] = pixVal;
				pix++;
			}
		}
	}

	//终止符
	lenTmp = pix;
	sign = 0;
	separator[2][0] = pix;
	for (int i = 0; i < 3; i++) {
		lenTmp += moduleWidth;
		for (; pix < lenTmp; pix++) {
			OutputData[pix] = sign;
		}
		sign = sign ^ 1;
	}
	separator[2][1] = pix;

	//右侧空白区
	lenTmp += 7 * moduleWidth;
	for (; pix < lenTmp; pix++) {
		OutputData[pix] = 1;
	}

	if (separatorIndex != nullptr) {
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 2; j++) {
				separatorIndex[(i << 1) + j] = separator[i][j];
			}
		}
	}
	return OutputLength;
}

inline int genEAN13Data(int moduleWidth){
	return (13 * moduleWidth);
}

int genEAN13Data(const char* ean13, int moduleWidth, int* OutputData, int OutputLength, int* separatorIndex, bool isContainCheck) {
	int eanLength = strlen(ean13);
	if ((isContainCheck == false && eanLength != 12) || (isContainCheck == true && eanLength != 13))return 0;
	return genEANData(ean13, moduleWidth, 6, OutputData, OutputLength, separatorIndex, isContainCheck, false);
}
