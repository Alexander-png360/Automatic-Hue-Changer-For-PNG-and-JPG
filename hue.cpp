#include "hue.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <filesystem>

int enterFile() {

    std::string fileLocation;

    std::cout << "Enter Directory: " << '\n';
    std::getline(std::cin, fileLocation);

    if (isFileValid(fileLocation)) {
        std::cout << "File is Valid PNG or JPG" << '\n';

        if (changeHueAll(fileLocation, 1)) {
            std::cout << "Hue Changed!" << '\n';
        }
        else {
            std::cout << "Hue Change Failed!" << '\n';
        }
    }
    else {
        std::cout << "Invalid File Type, must be PNG or JPG" << '\n';
    }

    return 0;
}

bool isFileValid(const std::string& fileLocation) {

    std::ifstream file(fileLocation, std::ios::binary);
    if (!file) return false;

    unsigned char header[8]{};
    file.read(reinterpret_cast<char*>(header), 8);

    const unsigned char pngSig[8] = { 0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A };
    bool isPng = true;

    for (int i = 0; i < 8; i++) {

        if (header[i] != pngSig[i]) {
            isPng = false;
            break;
        }
    }

    bool isJpg = header[0] == 0xFF && header[1] == 0xD8;

    return isPng || isJpg;

}

bool changeHueOnce(const std::string& filePath, int hueShift){

    cv::Mat img = cv::imread(filePath);
    if (img.empty()){
        std::cout << "Failed to load image" << '\n';
        return false;
    }

    //Convert to HSV
    cv::Mat hsv;
    cv::cvtColor(img, hsv, cv::COLOR_BGR2HSV);

    //Split Channels
    std::vector<cv::Mat> channels;
    cv::split(hsv, channels);

    //Hue Shift
    channels[0].forEach<uchar>([hueShift](uchar& pixel, const int*) {
        pixel = (pixel + (hueShift % 180))% 180;
        });

    // Merge HSV channels and convert back to BGR
    cv::merge(channels, hsv);
    cv::Mat output;
    cv::cvtColor(hsv, output, cv::COLOR_HSV2BGR);

    //Create Output
    std::filesystem::path p(filePath);
    std::string newName = p.stem().string() + "_hue" + p.extension().string();
    std::filesystem::path outPath = p.parent_path() / newName;

    //Save Image
    if (cv::imwrite(outPath.string(), output)) {
        std::cout << "Saved hue-changed image as: " << outPath.string() << '\n';
        return true;
    }

    std::cout << "Failed to save new image.\n";
    return false;
}

bool changeHueAll(const std::string& filePath, int step) {

    cv::Mat img = cv::imread(filePath);
    if (img.empty()){
        std::cout << "Failed to load image" << '\n';
        return false;
    }

    cv::Mat hsv;
    cv::cvtColor(img, hsv, cv::COLOR_BGR2HSV);

    //Split Channels
    std::vector<cv::Mat> channels;
    cv::split(hsv, channels);

    std::filesystem::path p(filePath);

    //set 180 

    for (int hueShift = 0; hueShift < 180; hueShift += step) {
        cv::Mat hueChannel = channels[0].clone();

        hueChannel.forEach<uchar>([hueShift](uchar& pixel, const int*) {
            pixel = (pixel + hueShift) % 180;
        });

        std::vector<cv::Mat> shiftedChannels = { hueChannel, channels[1], channels[2] };
        cv::Mat shiftedHSV;
        cv::merge(shiftedChannels, shiftedHSV);

        cv::Mat output;
        cv::cvtColor(shiftedHSV, output, cv::COLOR_HSV2BGR);


        std::string newName = p.stem().string() + "_hue_" + std::to_string(hueShift) + p.extension().string();
        std::filesystem::path outPath = p.parent_path() / newName;

        if (!cv::imwrite(outPath.string(), output)){
            std::cout << "Failed to save: " << outPath.string() << '\n';
        }
        else {
            std::cout << "Saved: " << outPath.string() << '\n';
        }
    }

    return true;

}