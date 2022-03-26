#include <iostream>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using std::endl;
using std::cout;
using std::cin;
using std::vector;
using std::string;
using std::stoi;
using std::sort;
using namespace cv;

#define THRESHOLD 240

cv::Point2d warmHouseDetector(cv::Mat &inputPhoto, cv::Mat &outputPhoto, int threshold) {

    // бинаризация изображения
    outputPhoto = inputPhoto.clone();
    cv::threshold(outputPhoto, outputPhoto, threshold, 255, cv::THRESH_BINARY);

    // изем контур цели
    std::vector< std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarh;
    cv::findContours(outputPhoto, contours, hierarh, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    int contourIdx = 0;
    for(size_t i = 1; i < contours.size(); i++) {
        if(contours[i].size() > contours[contourIdx].size()) {
            contourIdx = i;
        }
    }

    // ищем "центр тяжести" цели на фотографии
    cv::Moments mnts = cv::moments(contours[contourIdx]);
    double cols = mnts.m10 / mnts.m00;
    double rows = mnts.m01 / mnts.m00;
    cv::Point2d target(cols, rows);

    // отрисовка
    cv::cvtColor(outputPhoto, outputPhoto, cv::COLOR_GRAY2RGB);
    cv::drawContours(outputPhoto, contours, contourIdx, cv::Scalar(0, 0, 255), 1);
    cv::putText(outputPhoto, "100;100", target + cv::Point2d(5,5), cv::FONT_HERSHEY_COMPLEX, 1., cv::Scalar(0, 0, 255));
    cv::circle(outputPhoto, target, 5, cv::Scalar(0, 0, 255),-1);
    return target;
}

int main(int argc, char* argv[]) {

    if(argc == 1) {
        std::cout << "Please, run programm with argument - path to photo." << std::endl;
        return 1;
    }

    std::string filePath = argv[1];
    cv::Mat inputPhoto = imread(filePath, IMREAD_GRAYSCALE);
    if(inputPhoto.empty()) {
        std::cout << "Photo is empty" << std::endl;
        return 1;
    }

    cv::imshow("Landscape photo", inputPhoto);
    waitKey();

    cv::Mat targetPhoto;
    cv::Point2d target = warmHouseDetector(inputPhoto, targetPhoto, THRESHOLD);

    cv::imshow("Landscape photo", targetPhoto);
    waitKey();

	return 0;
}
