#pragma once

#include <opencv2/opencv.hpp>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "limits.h"
#include "../IThreadLauncher.h"
#include "../ILogger.h"
#include "LicensePlate.h"
#include "../Config.h"

class CalibParams : public ILogger {
public:
    CalibParams(const std::string &serverIp, const std::string &cameraIp);

    bool isLicensePlateInSelectedArea(const std::shared_ptr<LicensePlate> &licensePlate, const std::string &maskType);

    void getMask();

    const std::string &getCameraIp() const;

private:
    const float FRAME_WIDTH_HD = 1920.0;
    const float FRAME_HEIGHT_HD = 1080.0;
    const int WHITE_COLOR = 255;

    std::string cameraIp;
    cv::Mat mask;
    cv::Mat mask2;
    std::vector<cv::Point2i> maskPoints;
    std::vector<cv::Point2i> subMaskPoints;
    std::string calibParamsUrl;
    std::mutex maskAccessChangeMutex;

    bool isPointInTheMask(const cv::Point2i &point);

    bool isPointInTheSubMask(const cv::Point2i &point);

    std::string sendRequestForMaskPoints();

    [[nodiscard]] std::vector<cv::Point2i> getDefaultPolygonPoints() const;

    [[nodiscard]] cv::Point2i getRelatedPoint(const cv::Point2f &point, const cv::Size &imageSize) const;

    static void showCenterPointInGround(const std::shared_ptr<LicensePlate> &licensePlate,
                                        const cv::Point2f &centerPointInGround);

    [[nodiscard]] std::vector<cv::Point2i> getPolygonPoints(const std::string &polygonPointsStr, const std::string &maskType) const;


};
