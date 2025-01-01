#pragma once

#include <string>

#include <opencv2/opencv.hpp>

#include "../ITimer.h"

class FrameData : public ITimer {
public:
    FrameData(std::string ip, cv::Mat frame, double rtpTimestamp,
              std::chrono::high_resolution_clock::time_point startTime);

    const std::string &getIp();

    const cv::Mat &getFrame();

    double getRTPtimestamp() const;

    float getFrameWidth() const;

    float getFrameHeight() const;

private:
    std::string ip;
    cv::Mat frame;
    double rtpTimestamp;
};