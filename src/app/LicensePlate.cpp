#include "LicensePlate.h"

#include <utility>

using namespace std;

LicensePlate::LicensePlate(int x, int y, int w, int h, int x1, int y1,
                           int x2, int y2, int x3, int y3, int x4, int y4) {

    width = (int) w;
    height = (int) h;
    center = cv::Point2i(x, y);
    leftTop = cv::Point2f(x1, y1);
    leftBottom = cv::Point2f(x2, y2);
    rightTop = cv::Point2f(x3, y3);
    rightBottom = cv::Point2f(x4, y4);

    if ((rightTop.x - leftTop.x) / (leftBottom.y - leftTop.y) < SQUARE_LP_RATIO) {
        square = true;
    }
}

void LicensePlate::setPlateImage(const cv::Mat &frame) {

    cv::Mat transformationMatrix;
    cv::Size lpSize;

    if (square) {
        transformationMatrix = cv::getPerspectiveTransform(vector<cv::Point2f>{
                leftTop, leftBottom, rightTop, rightBottom
        }, Constants::SQUARE_LP_COORS);

        lpSize = cv::Size(Constants::SQUARE_LP_W, Constants::SQUARE_LP_H);

    } else {
        transformationMatrix = cv::getPerspectiveTransform(vector<cv::Point2f>{
                leftTop, leftBottom, rightTop, rightBottom
        }, Constants::RECT_LP_COORS);

        lpSize = cv::Size(Constants::RECT_LP_W, Constants::RECT_LP_H);
    }
    cv::warpPerspective(frame, plateImage, transformationMatrix, lpSize);
}

const cv::Mat &LicensePlate::getPlateImage() const {
    return plateImage;
}

const cv::Point2i &LicensePlate::getCenter() const {
    return center;
}

const cv::Point2f &LicensePlate::getLeftTop() const {
    return leftTop;
}

const cv::Point2f &LicensePlate::getRightBottom() const {
    return rightBottom;
}


const cv::Point2f &LicensePlate::getLeftBottom() const {
    return leftBottom;
}

const cv::Point2f &LicensePlate::getRightTop() const {
    return rightTop;
}

bool LicensePlate::isSquare() const {
    return square;
}

const string &LicensePlate::getPlateLabel() const {
    return licensePlateLabel;
}

void LicensePlate::setLicensePlateLabel(string lpLabel) {
    licensePlateLabel = std::move(lpLabel);
}

void LicensePlate::setCameraIp(string ip) {
    cameraIp = std::move(ip);
}

const string &LicensePlate::getCameraIp() const {
    return cameraIp;
}

void LicensePlate::setCarImage(cv::Mat image) {
    carImage = std::move(image);
}

const cv::Mat &LicensePlate::getCarImage() const {
    return carImage;
}

void LicensePlate::setRTPtimestamp(double timestamp) {
    rtpTimestamp = timestamp;
}

double LicensePlate::getRTPtimestamp() const {
    return rtpTimestamp;
}

cv::Size LicensePlate::getCarImageSize() const {
    return carImageSize;
}

void LicensePlate::setCarImageSize(cv::Size size) {
    carImageSize = std::move(size);
}



