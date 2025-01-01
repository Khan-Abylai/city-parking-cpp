#pragma once

#include <thread>

#include "../SharedQueue.h"
#include "../ILogger.h"
#include "../IThreadLauncher.h"
#include "GstreamerReader.h"

class CameraClientLauncher : public IThreadLauncher, public ILogger {
public:
    CameraClientLauncher(const std::vector<std::string> &cameraIps,
                         const std::vector<std::shared_ptr<SharedQueue<std::shared_ptr<FrameData>>>> &frameQueues);

    void run() override;

    void shutdown() override;

protected:
    std::vector<std::shared_ptr<GstreamerReader>> cameraReaders;
    std::vector<std::thread> threads;
};