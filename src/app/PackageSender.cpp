#include "PackageSender.h"

using namespace std;


PackageSender::PackageSender(const string &serverIp,
                             shared_ptr<SharedQueue<shared_ptr<Package>>> packageQueue)
        : ILogger("Package Sender --------------------"),
          packageQueue{std::move(packageQueue)} {

    serverUrl = serverIp;
    LOG_INFO("package sender server url %s", serverUrl.data());
}

void PackageSender::run() {
    time_t beginTime = time(nullptr);
    queue<cpr::AsyncResponse> futureResponses;
    while (!shutdownFlag) {
        auto package = packageQueue->wait_and_pop();
        if (package == nullptr) continue;

        LOG_INFO("%s %s %s", package->getCameraIp().data(), package->getPlateLabel().data(),Utils::dateTimeToStr(time_t(nullptr)).c_str());

        futureResponses.push(sendRequests(package->getPackageJsonString()));


        while (futureResponses.size() > MAX_FUTURE_RESPONSES) {
            futureResponses.pop();
        }
    }
}


void PackageSender::shutdown() {
    LOG_INFO("service is shutting down");
    shutdownFlag = true;
    packageQueue->push(nullptr);
}

cpr::AsyncResponse PackageSender::sendRequests(const string &jsonString) {
    return cpr::PostAsync(
            cpr::Url{serverUrl},
            cpr::VerifySsl(false),
            cpr::Body{jsonString},
            cpr::Timeout{SEND_REQUEST_TIMEOUT},
            cpr::Header{{"Content-Type", "application/json"}});
}



