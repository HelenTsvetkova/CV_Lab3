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

static cv::Mat inputPhoto;
static int thresholdMin = 240;
static int thresholdMax = 256;


static void trackbar_handler(int, void*) {
    if(thresholdMin > thresholdMax) {
        cout << "Threshold minimum is bigger than threshold maximum.\n";
        return;
    }

    // Отфильтровываем только то, что нужно, по пороговому значнию
    double thrMin = (double)thresholdMin;
    double thrMax = (double)thresholdMax;
    cv::Mat outputPhoto;
    cv::threshold(inputPhoto, outputPhoto, thrMin, thrMax, cv::THRESH_BINARY);

    // Визуально выделяем границы
    cv::imshow("Settings Threshold", outputPhoto);
}

cv::Point2d warmHouseDetector(cv::Mat &inputPhoto, cv::Mat &targetPhoto, int threshold) {

    // бинаризация изображения
    cv::threshold(inputPhoto, targetPhoto, threshold, 255., cv::THRESH_BINARY);
    // ищем контур цели
    std::vector< std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarh;
    cv::findContours(targetPhoto, contours, hierarh, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    int maxContourIdx = 0;
    double maxContourArea = 0.0;
    for(size_t i = 0; i < contours.size(); i++) {
        double area = cv::contourArea(contours[i]);
        if(area > maxContourArea) {
            maxContourIdx = i;
            maxContourArea = area;
        }
    }

    // ищем "центр тяжести" цели на фотографии
    cv::Moments mnts = cv::moments(contours[maxContourIdx]);
    double cols = mnts.m10 / mnts.m00;
    double rows = mnts.m01 / mnts.m00;
    cv::Point2d target(cols, rows);

    // отрисовка
    cv::cvtColor(targetPhoto, targetPhoto, cv::COLOR_GRAY2RGB);
    cv::drawContours(targetPhoto, contours, maxContourIdx, cv::Scalar(0, 0, 255), 3);
    cv::putText(targetPhoto, std::to_string(cols) + " : " + std::to_string(rows), target + cv::Point2d(20,-20), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 255));
    cv::circle(targetPhoto, target, 5, cv::Scalar(0, 0, 255),-1);
    return target;
}

int main(int argc, char* argv[]) {

    if(argc == 1) {
        std::cout << "Please, run programm with argument - path to photo." << std::endl;
        return 1;
    }

    std::string filePath = argv[1];
    inputPhoto = imread(filePath, cv::IMREAD_GRAYSCALE);
    if(inputPhoto.empty()) {
        std::cout << "Photo is empty" << std::endl;
        return 1;
    }

    cv::imshow("photo", inputPhoto);
    cv::waitKey();

    // Создаём ползунки для последующей коррекции параметров HSV
    namedWindow("Settings Threshold", cv::WINDOW_NORMAL);
    cv::createTrackbar("Threshold from", "Settings Threshold", &thresholdMin, 256, trackbar_handler);
    cv::createTrackbar("Threshold to ", "Settings Threshold", &thresholdMax, 256, trackbar_handler);

    // Инициализируем обработку параметров HSV
    trackbar_handler(0,0);
    // Дальнейший цикл - пока пользователь меняет положение трекеров и/или НЕ нажимает на esc
    while(true) {
        int iKey = cv::waitKey(50);
        if (iKey == 27) {
            break;
        }
    }

    cv::Mat targetPhoto;
    cv::Point2d target = warmHouseDetector(inputPhoto, targetPhoto, thresholdMin);
    std::cout << "Target : " << target << std::endl;
    cv::imshow("Target photo", targetPhoto);
    waitKey();

	return 0;
}
