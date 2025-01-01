#pragma once

#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>

#include "Utils.h"
#include "LicensePlate.h"
#include "../ITimer.h"
#include "../Config.h"

class Package : public ITimer {
public:
    Package(std::string cameraIp, std::string licensePlateLabel,
            cv::Mat carImage);

    [[nodiscard]] std::string getPackageJsonString() const;
    [[nodiscard]] const std::string &getPlateLabel() const;

    [[nodiscard]] const std::string &getCameraIp() const;


private:
    std::string cameraIp;
    std::string licensePlateLabel;
    cv::Mat carImage;
};

