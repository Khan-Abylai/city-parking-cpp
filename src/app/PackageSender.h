#pragma once

#include <ctime>
#include <future>
#include <unordered_map>

#include <cpr/cpr.h>

#include "../IThreadLauncher.h"
#include "../ILogger.h"
#include "../SharedQueue.h"
#include "LicensePlate.h"
#include "Package.h"
#include "Utils.h"

class PackageSender : public IThreadLauncher, public ILogger {
public:
    PackageSender(const std::string &serverIp,
                  std::shared_ptr<SharedQueue<std::shared_ptr<Package>>> packageQueue);

    void run() override;

    void shutdown() override;

private:

    const int SEND_REQUEST_TIMEOUT = 10000;
    const int MAX_FUTURE_RESPONSES = 30;

    double avgDetectionTime = 0;
    double avgRecognizerTime = 0;
    double avgOverallTime = 0;

    double maxDetectionTime = -1.0;
    double maxRecognizerTime = -1.0;
    double maxOverallTime = -1.0;

    int iteration = 0;

    std::unordered_map<std::string, std::time_t> lastSendTimes;
    std::string serverUrl;
    std::shared_ptr<SharedQueue<std::shared_ptr<Package>>> packageQueue;

    cpr::AsyncResponse sendRequests(const std::string &jsonString);

};