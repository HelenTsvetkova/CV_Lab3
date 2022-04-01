#include <iostream>
#include <string>
#include <map>
#include <set>

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

void detectDetail(cv::Mat & detailsImage, cv::Mat & patternImage, std::vector< std::vector<cv::Point> > & details,
                  std::vector< std::vector<cv::Point> > & defectDetails) {

    // контуры паттерна
    cv::Mat patternGray = patternImage.clone();
    cv::cvtColor(patternGray, patternGray, cv::COLOR_BGR2GRAY);
    cv::threshold(patternGray, patternGray, 240., 256., cv::THRESH_BINARY);
    std::vector< std::vector<cv::Point> > contoursBuf;
    cv::findContours(patternGray, contoursBuf, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    // фильтруем контуры, находим предполагаемый контур паттерна
    int minPoints = 20; //принимаем минимальное количество углов у контура детали
    int patternContourIdx = -1;
    double maxArea = 0.0;
    for(size_t i = 0; i < contoursBuf.size(); i++) {
        if(contoursBuf[i].size() < minPoints) {
            continue;
        }
        double area = cv::contourArea(contoursBuf[i]);
        if(area > maxArea) {
            maxArea = area;
            patternContourIdx = i;
        }
    }

    if(patternContourIdx == -1) {
        std::cout << "Error detecting pattern contour" << std::endl;
        return;
    }
    std::vector<cv::Point> contourPattern = contoursBuf[patternContourIdx];

    // ищем контуры на фото с деталями
    cv::Mat detailsGray = detailsImage.clone();
    cv::cvtColor(detailsGray, detailsGray, cv::COLOR_BGR2GRAY);
    cv::threshold(detailsGray, detailsGray, 240., 256., cv::THRESH_BINARY);
    contoursBuf.clear();
    cv::findContours(detailsGray, contoursBuf, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    // фильтруем контуры по количеству точек
    std::vector< std::vector<cv::Point> > contoursDetails;
    for(size_t i = 0; i < contoursBuf.size(); i++) {
        if(contoursBuf[i].size() > minPoints) {
            contoursDetails.push_back(contoursBuf[i]);
        }
    }

    // фильтруем контуры, сравнивая их с контуром паттерна
    double thresholdMax = 4.0; // мера для определения деталей отдалённо напоминающих паттерн
    double thresholdMin = 0.8; // мера для определения деталей сильно напоминающих паттерн
    for(size_t i = 0; i < contoursDetails.size(); i++) {
        double matchResult = cv::matchShapes(contourPattern,contoursDetails[i], cv::CONTOURS_MATCH_I2, 0);
        if(matchResult < thresholdMax) {
            if(matchResult > thresholdMin) {
                defectDetails.push_back(contoursDetails[i]);
            } else {
                details.push_back(contoursDetails[i]);
            }
        }
    }
}

int main(int argc, char* argv[]) {

    if(argc == 1) {
        std::cout << "Please, run programm with argument - path to photo and path to pattern." << std::endl;
        return 1;
    }

    std::string filePath = argv[1];
    cv::Mat detailsImage = imread(filePath);
    if(detailsImage.empty()) {
        std::cout << "Photo is empty" << std::endl;
        return 1;
    }

    filePath = argv[2];
    cv::Mat patternImage = imread(filePath);
    if(patternImage.empty()) {
        std::cout << "Pattern photo is empty" << std::endl;
        return 1;
    }

    std::vector< std::vector<cv::Point> >  detailsContours;
    std::vector< std::vector<cv::Point> >  defectDetailsContours;
    detectDetail(detailsImage, patternImage, detailsContours, defectDetailsContours);

    for(size_t i = 0; i < detailsContours.size(); i++) {
        cv::drawContours(detailsImage, detailsContours, i, cv::Scalar(0, 255, 0), 3);
        cv::putText(detailsImage, std::string("detail ") + std::to_string(i), detailsContours[i][0], cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 255, 0));
    }

    for(size_t i = 0; i < defectDetailsContours.size(); i++) {
        cv::drawContours(detailsImage, defectDetailsContours, i, cv::Scalar(0, 0, 255), 3);
        cv::putText(detailsImage, std::string("defect ") + std::to_string(i), defectDetailsContours[i][0], cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 255));
    }

    imshow("Defect details image", detailsImage);
    waitKey();

    return 0;
}
