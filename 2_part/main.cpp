#include <iostream>
#include <string>
#include <map>

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
static cv::Mat outputPhoto;
static int Hmin = 0;
static int Hmax = 256;
static int Smin = 0;
static int Smax = 256;
static int Vmin = 83;
static int Vmax = 256;
static int HSVmax = 256;

static void trackbar_handler(int, void*) {
    if(Hmin > Hmax) {
        cout << "Hue minimum is bigger than hue maximum.\n";
        return;
    } else if(Smin > Smax) {
        cout << "Saturation minimum is bigger than Saturation maximum.\n";
        return;
    } else if(Vmin > Vmax) {
        cout << "Saturation minimum is bigger than Saturation maximum.\n";
        return;
    }
    // Отфильтровываем только то, что нужно, по диапазону цветов
    cv::inRange(
        inputPhoto,
        cv::Scalar( Hmin, Smin, Vmin ),
        cv::Scalar( Hmax, Smax, Vmax ),
        outputPhoto
    );

    // Визуально выделяем границы
    cv::imshow("Settings", outputPhoto);
}

std::vector<cv::Point2d> detectPlane() {

    cv::threshold(outputPhoto, outputPhoto, 240., 256., cv::THRESH_BINARY);

    // ищем контур цели
    std::vector< std::vector<cv::Point> > contours;
    std::vector<cv::Point2d> targets;
    cv::findContours(outputPhoto, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    std::map<double, int> contourAreas;
    for(size_t i = 1; i < contours.size(); i++) {
        double area = cv::contourArea(contours[i]);
        double procentPart = 100. * ( area / (inputPhoto.cols*inputPhoto.rows));
        if(procentPart > 0.05) {
            contourAreas[area] = i;
            cv::Moments mnts = cv::moments(contours[i]);
            double cols = mnts.m10 / mnts.m00;
            double rows = mnts.m01 / mnts.m00;
            targets.push_back(cv::Point2d(cols, rows));
        }
    }

    cv::cvtColor(outputPhoto, outputPhoto, cv::COLOR_GRAY2RGB);

    int targetIdx = 0;
    for(auto contourSizes_it = contourAreas.begin(); contourSizes_it != contourAreas.end(); contourSizes_it++) {
        int contourIdx = contourSizes_it->second;
        cv::drawContours(outputPhoto, contours, contourIdx, cv::Scalar(0, 0, 255), 1);
        cv::putText(outputPhoto, std::string("point ") + std::to_string(targetIdx), targets[targetIdx] + cv::Point2d(5,5), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 255));
        cv::circle(outputPhoto, targets[targetIdx], 5, cv::Scalar(0, 0, 255),-1);
        targetIdx++;
    }
    return targets;
}

int main(int argc, char* argv[]) {

    if(argc == 1) {
        std::cout << "Please, run programm with argument - path to photo." << std::endl;
        return 1;
    }

    std::string filePath = argv[1];
    inputPhoto = imread(filePath);
    if(inputPhoto.empty()) {
        std::cout << "Photo is empty" << std::endl;
        return 1;
    }

    outputPhoto = inputPhoto.clone();
    cv::cvtColor(inputPhoto, outputPhoto, cv::COLOR_RGB2GRAY);

    // Создаём ползунки для последующей коррекции параметров HSV
    namedWindow("Settings", cv::WINDOW_NORMAL);
    cv::createTrackbar("Hue from","Settings", &Hmin, HSVmax,trackbar_handler);
    cv::createTrackbar("hue to ","Settings", &Hmax, HSVmax,trackbar_handler);
    cv::createTrackbar("Saturation from","Settings",&Smin, HSVmax,trackbar_handler);
    cv::createTrackbar("saturation to","Settings",&Smax, HSVmax,trackbar_handler);
    cv::createTrackbar("Value from","Settings", &Vmin, HSVmax,trackbar_handler);
    cv::createTrackbar("Value to","Settings", &Vmax, HSVmax,trackbar_handler);

    // Инициализируем обработку параметров HSV
    trackbar_handler(0,0);
    // Дальнейший цикл - пока пользователь меняет положение трекеров и/или НЕ нажимает на esc
    while(true) {
        int iKey = cv::waitKey(50);
        if (iKey == 27) {
            break;
        }
    }

    std::vector<cv::Point2d> targets = detectPlane();
    cv::namedWindow("detection", cv::WINDOW_FREERATIO);
    cv::imshow("detection", outputPhoto);
    cv::waitKey();

    for(size_t i = 0; i < targets.size(); i++) {
        std::cout << "Target's " << i << " coordinates : " << targets[i] << std::endl;
    }

	return 0;
}
