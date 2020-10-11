#include <iostream>
#include <vector>
#include "EANpeX.h"

std::vector<std::string> linesFromFile(std::string path);

int main() {
	std::string ean;
	std::vector<std::string> strdata = linesFromFile("LightIntensity.txt");
	int dataSize = strdata.size() - 1;
	int thresholdValue = 128;
	int* pix = (int*)malloc(dataSize * sizeof(int));
	int* val = (int*)malloc(dataSize * sizeof(int));

	//read light intensity data from file.
	for (int i = 1; i <= dataSize; i++) {
		sscanf(strdata[i].c_str(), "%d%d", &pix[i - 1], &val[i - 1]);
	}

	//threshold data.
	threshold(val, dataSize, thresholdValue);

	//Parse
	int isParse = parseEAN_DEC(val, dataSize, -1, ean);

	//Output
	if (isParse == 0)std::cout << "Parse succeeded: " << ean << std::endl;
	else std::cout << "Parse failed." << std::endl;

	return 0;
}

std::vector<std::string> linesFromFile(std::string path) {
	std::ifstream rder;
	std::vector<std::string> output;
	std::string tmp;
	rder.open(path);
	if (rder.is_open()) {
		while (rder.peek() != EOF) {
			std::getline(rder, tmp);
			output.push_back(tmp);
		}
	}
	return output;
}
