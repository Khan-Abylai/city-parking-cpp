#include "CameraClientLauncher.h"

using namespace std;

CameraClientLauncher::CameraClientLauncher(const vector<string> &cameraIps,
                                           const vector<shared_ptr<SharedQueue<shared_ptr<FrameData>>>> &frameQueues)
                                           : ILogger("Camera Client Launcher ") {

    for (int i = 0; i < cameraIps.size(); i++) {
        auto cameraReader = make_shared<GstreamerReader>(cameraIps[i], frameQueues[i]);
        cameraReaders.push_back(std::move(cameraReader));
    }
}

void CameraClientLauncher::run() {
    for (const auto &gstreamer : cameraReaders)
        threads.emplace_back(&GstreamerReader::launchStream, gstreamer);
}

void CameraClientLauncher::shutdown() {
    for (int i = 0; i < cameraReaders.size(); i++) {
        cameraReaders[i]->shutdown();
        if (threads[i].joinable())
            threads[i].join();
    }
}
