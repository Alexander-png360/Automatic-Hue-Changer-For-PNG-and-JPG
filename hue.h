#pragma once
#include <string>

int enterFile();

bool isFileValid(const std::string& fileLocation);

bool changeHueOnce(const std::string& filePath, int hueShift = 30);
bool changeHueAll(const std::string& filePath, int step = 10);