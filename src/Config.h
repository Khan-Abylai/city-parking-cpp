#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>

#include "app/Utils.h"

class Config {
public:
    static bool parseJson(const std::string &fileName);

    static const std::vector<std::string> &getCameraIps();

    static const std::string &getResultSendIp();

    static const std::string &getNodeIp();

    static bool useGPUDecode();

    static const int &getPlatesCount();

    static const int &getTimeSentPlates();

    static const float &getRecognizerThreshold();

    static const float &getDetectorThreshold();

};