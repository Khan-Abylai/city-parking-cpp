#pragma once

#include <chrono>

#include "../IThreadLauncher.h"
#include "../ILogger.h"
#include "../SharedQueue.h"
#include "../client/FrameData.h"

#include "../RandomStringGenerator.h"
#include "CarTracker.h"
#include "CalibParamsUpdater.h"
#include "TemplateMatching.h"
#include "Package.h"
#include "../anpr/DetectionNCNN.h"
#include "../anpr/LPRecognizerNCNN.h"

class FrameProcessor : public IThreadLauncher, public ::ILogger {
public:
    FrameProcessor(const std::shared_ptr<SharedQueue<std::shared_ptr<Package>>> &packageQueue,
                   std::shared_ptr<SharedQueue<std::shared_ptr<FrameData>>> frameQueue,
                   std::string &cameraIp,
                   std::string &nodeIp,int platesCount);

    void run() override;

    void shutdown() override;

private:

    float RECOGNIZER_THRESHOLD = 0.8;
    static bool isChooseThisFrame();

    cv::Mat whiteImage = cv::Mat(Constants::RECT_LP_H, Constants::RECT_LP_W, CV_8UC3, cv::Scalar(255, 255, 255));
    std::shared_ptr<CarTracker> carTracker;
    std::shared_ptr<SharedQueue<std::shared_ptr<FrameData>>> frameQueue;

    static void saveFrame(const std::shared_ptr<LicensePlate> &plate);

    static std::vector<cv::Mat> getLicensePlateImages(const std::shared_ptr<LicensePlate> &licensePlate);

    static void showFrame(const std::vector<std::shared_ptr<LicensePlate>> &plates, const cv::Mat &frame);
    static cv::Mat combineInOneLineSquareLpPlate(const cv::Mat &lpImage);

    std::shared_ptr<DetectionNCNN> detectionNCNN;
    std::unique_ptr<LPRecognizerNCNN> lpRecognizerNCNN;

    std::unique_ptr<TemplateMatching> templateMatching;
    std::shared_ptr<CalibParams> calibParams2;
    std::unique_ptr<CalibParamsUpdater> calibParamsUpdater2;

    std::vector<std::shared_ptr<LicensePlate>>
    filterOutOfMaskLicensePlates(std::vector<std::shared_ptr<LicensePlate>> &allLicensePlates,
                                 std::shared_ptr<FrameData> &frameData);
    std::vector<std::shared_ptr<LicensePlate>> recognize(const std::vector<std::shared_ptr<LicensePlate>>& licensePlates);

    std::pair<std::string, float>
    getLicensePlateLabel(const std::vector<std::pair<std::string, float>> &recognizerResult, bool isSquarePlate);
    bool isValidLicensePlate(const std::string &lpLabel, float probability);

};


