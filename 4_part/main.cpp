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

void detectDetail(cv::Mat & inputImage, cv::Mat & detailPattern, std::vector< std::vector<cv::Point> > & details,
                  std::vector< std::vector<cv::Point> > & defectDetails) {

    // контуры паттерна
    cv::Mat patternGray = detailPattern.clone();
    cv::cvtColor(patternGray, patternGray, cv::COLOR_BGR2GRAY);
    std::vector< std::vector<cv::Point> > contoursPattern;
    cv::findContours(patternGray, contoursPattern, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    int patternContourIdx = 0;
    double maxArea = 0.0;
    for(size_t i = 0; i < contoursPattern.size(); i++) {
        double area = cv::contourArea(contoursPattern[i]);
        if(area > maxArea) {
            maxArea = area;
            patternContourIdx = i;
        }
    }

    // предобработка фото
    cv::Mat contourImage = inputImage.clone();
    cv::cvtColor(contourImage, contourImage, cv::COLOR_BGR2GRAY);
    cv::threshold(contourImage, contourImage, 240., 256., cv::THRESH_BINARY);

    // ищем контуры на фото с деталями
    std::vector< std::vector<cv::Point> > contours;
    cv::findContours(contourImage, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    // сравнение контуров
    std::multimap<double, int> contoursMatchResults;
    for(size_t i = 0; i < contours.size(); i++) {
        double matchResult = cv::matchShapes(
                    contoursPattern[patternContourIdx],
                    contours[i], cv::CONTOURS_MATCH_I2, 0
                    );
        contoursMatchResults.insert(std::pair<double, int>(matchResult, i));
    }

    for(auto contourIt = contoursMatchResults.begin(); contourIt != contoursMatchResults.end(); contourIt++) {
        if(contourIt->first > 0.8) {
            defectDetails.push_back(contours[contourIt->second]);
        } else {
            details.push_back(contours[contourIt->second]);
        }
    }


//    // обработка контуров
//    std::multimap<double, int> contourAreas;
//    for(size_t i = 0; i < contours.size(); i++) {
//        double area = cv::contourArea(contours[i]);
//        contourAreas.insert(std::pair<double, int>(area, i));
//    }

//    // вывод на фото
//    int targetIdx = 0;
//    for(auto contourSizes_it = contourAreas.rbegin(); contourSizes_it != contourAreas.rend(); contourSizes_it++) {
//        int contourIdx = contourSizes_it->second;
//        cv::drawContours(inputImage, contours, contourIdx, cv::Scalar(0, 0, 255), 1);
//        cv::putText(inputImage, std::string("point ") + std::to_string(targetIdx), contours[contourIdx][0], cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 255));
//        targetIdx++;
//    }
//    cv::imshow("corners", inputImage);
//    cv::waitKey();

}

int main(int argc, char* argv[]) {

    if(argc == 1) {
        std::cout << "Please, run programm with argument - path to photo and path to pattern." << std::endl;
        return 1;
    }

    std::string filePath = argv[1];
    cv::Mat inputPhoto = imread(filePath);
    if(inputPhoto.empty()) {
        std::cout << "Photo is empty" << std::endl;
        return 1;
    }

    filePath = argv[2];
    cv::Mat patternPhoto = imread(filePath);
    if(patternPhoto.empty()) {
        std::cout << "Pattern photo is empty" << std::endl;
        return 1;
    }

    std::vector< std::vector<cv::Point> >  detailsCont;
    std::vector< std::vector<cv::Point> >  defectDetailsCont;
    detectDetail(inputPhoto, patternPhoto, detailsCont, defectDetailsCont);

    for(size_t i = 0; i < detailsCont.size(); i++) {
        cv::drawContours(inputPhoto, detailsCont, i, cv::Scalar(0, 0, 255), 3);
    }

    imshow("inputPhoto", inputPhoto);
    waitKey();

    return 0;
}
