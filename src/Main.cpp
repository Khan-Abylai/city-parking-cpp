#include <thread>
#include <chrono>
#include <csignal>
#include <condition_variable>
#include <mutex>
#include "client/CameraClientLauncher.h"
#include "Config.h"
#include "SharedQueue.h"
#include "app/FrameProcessor.h"
#include "app/PackageSender.h"
#include "app/Package.h"

using namespace std;

atomic<bool> shutdownFlag = false;
condition_variable shutdownEvent;
mutex shutdownMutex;

void signalHandler(int signum) {
    cout << "signal is to shutdown" << endl;
    shutdownFlag = true;
    shutdownEvent.notify_all();
}

int main(int argc, char *argv[]) {


    string configFileName;

    if (argc <= 1)
        configFileName = "./config.json";
    else
        configFileName = argv[1];

    if (!Config::parseJson(configFileName))
        return -1;

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGKILL, signalHandler);
    signal(SIGHUP, signalHandler);
    signal(SIGABRT, signalHandler);

    auto packageQueue = make_shared<SharedQueue<shared_ptr<Package>>>();
    vector<shared_ptr<IThreadLauncher>> services;
    auto cameraIps = Config::getCameraIps();
    vector<shared_ptr<SharedQueue<shared_ptr<FrameData>>>> frameQueues;

    string nodeIp = Config::getNodeIp();
    for (auto &cameraIp: cameraIps) {
        auto frameQueue = make_shared<SharedQueue<shared_ptr<FrameData>>>();
        auto parking = make_shared<FrameProcessor>(packageQueue,frameQueue,
                                                   cameraIp,
                                                   nodeIp,  Config::getPlatesCount());
        services.emplace_back(parking);
        frameQueues.push_back(std::move(frameQueue));
    }

    shared_ptr<IThreadLauncher> clientStarter;
    clientStarter = make_shared<CameraClientLauncher>(cameraIps, frameQueues);
    services.emplace_back(clientStarter);


    auto packageSender = make_shared<PackageSender>(Config::getResultSendIp(), packageQueue);
    services.emplace_back(packageSender);

    vector<thread> threads;
    for (const auto &service: services) {
        threads.emplace_back(&IThreadLauncher::run, service);
    }

    unique_lock<mutex> shutdownLock(shutdownMutex);
    while (!shutdownFlag) {
        auto timeout = chrono::hours(24);
        if (shutdownEvent.wait_for(shutdownLock, timeout, [] { return shutdownFlag.load(); })) {
            cout << "shutting all services" << endl;
        }
    }

    for (int i = 0; i < services.size(); i++) {
        services[i]->shutdown();
        if (threads[i].joinable())
            threads[i].join();
    }
}
