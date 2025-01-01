#include "FrameProcessor.h"

using namespace std;

FrameProcessor::FrameProcessor(const std::shared_ptr<SharedQueue<std::shared_ptr<Package>>> &packageQueue,
                               shared_ptr<SharedQueue<shared_ptr<FrameData>>> frameQueue,
                               string &cameraIp, string &nodeIp, int platesCount) : ILogger("FrameProcessor "),
                                                                       frameQueue{std::move(frameQueue)} {
    calibParams2 = make_shared<CalibParams>(nodeIp, cameraIp);
    calibParamsUpdater2 = make_unique<CalibParamsUpdater>(calibParams2);
    calibParamsUpdater2->run();
    calibParams2->getMask();

    carTracker = make_shared<CarTracker>(calibParams2, platesCount, packageQueue);
    templateMatching = make_unique<TemplateMatching>();
    detectionNCNN = make_shared<DetectionNCNN>();
    lpRecognizerNCNN = make_unique<LPRecognizerNCNN>();
};


void FrameProcessor::run() {
    while (!shutdownFlag) {
        auto frameData = frameQueue->wait_and_pop();
        if (frameData == nullptr) continue;
        auto frame = frameData->getFrame();

        auto startTime = chrono::high_resolution_clock::now();
        auto licensePlates = detectionNCNN->detect(frame);
        auto endTime = chrono::high_resolution_clock::now();

        if (licensePlates.empty()) continue;

        licensePlates = filterOutOfMaskLicensePlates(licensePlates, frameData);
        auto stop = 1;
        if (licensePlates.empty()) continue;

        auto startRecognizerTime = chrono::high_resolution_clock::now();
        licensePlates = recognize(licensePlates);
        auto endRecognizeTime = chrono::high_resolution_clock::now();

        licensePlates.erase(
                std::remove_if(
                        licensePlates.begin(), licensePlates.end(),
                        [](const std::shared_ptr<LicensePlate> &lp) {
                            return lp->getPlateLabel() == "NOT_RECOGNIZED";
                        }
                ), licensePlates.end()
        );
        if (licensePlates.empty())
            continue;

        carTracker->track(frameData, licensePlates);
        carTracker->sentPackages();
        auto oldCars = carTracker->removeOldCars();
    }
}

void FrameProcessor::shutdown() {
    LOG_INFO("service is shutting down");
    shutdownFlag = true;
    frameQueue->push(nullptr);
}

bool FrameProcessor::isChooseThisFrame() {
    srand(time(nullptr));
    auto randomNumber = 1 + rand() % 100; // generating number between 1 and 100
    return (randomNumber < 20);
}


void FrameProcessor::saveFrame(const shared_ptr<LicensePlate> &plate) {
    if (!isChooseThisFrame()) return;
    string fileName = RandomStringGenerator::generate(30, Constants::IMAGE_DIRECTORY, Constants::JPG_EXTENSION);
    auto frame = plate->getCarImage();
    auto frame_plate = plate->getPlateImage();
    cv::imwrite(fileName, frame_plate);
}

std::vector<shared_ptr<LicensePlate>>
FrameProcessor::filterOutOfMaskLicensePlates(std::vector<std::shared_ptr<LicensePlate>> &allLicensePlates,
                                             shared_ptr<FrameData> &frameData) {
    vector<shared_ptr<LicensePlate>> licensePlatesOnMask;

    for (auto &lp: allLicensePlates) {
        lp->setCarImageSize(frameData->getFrame().size());
        lp->setPlateImage(frameData->getFrame());
        lp->setCameraIp(frameData->getIp());
        lp->setRTPtimestamp(frameData->getRTPtimestamp());
        lp->setCarImage(frameData->getFrame());
    }

    copy_if(allLicensePlates.begin(), allLicensePlates.end(), back_inserter(licensePlatesOnMask),
            [this](const shared_ptr<LicensePlate> &lp) {
                return calibParams2->isLicensePlateInSelectedArea(lp, "main");
            });
    return std::move(licensePlatesOnMask);
}

void FrameProcessor::showFrame(const vector<std::shared_ptr<LicensePlate>> &plates, const cv::Mat &frame) {
    cv::Mat clone = frame.clone();
    for (const auto &lp: plates) {
        circle(clone, lp->getCenter(), 1, cv::Scalar(255, 0, 0), 5, 8, 0);
        circle(clone, lp->getLeftTop(), 1, cv::Scalar(255, 255, 0), 5, 8, 0);
        circle(clone, lp->getRightTop(), 1, cv::Scalar(255, 255, 0), 5, 8, 0);
        circle(clone, lp->getLeftBottom(), 1, cv::Scalar(255, 255, 0), 5, 8, 0);
        circle(clone, lp->getRightBottom(), 1, cv::Scalar(255, 255, 0), 5, 8, 0);
    }
    cv::Mat showImage;

    cv::resize(clone, showImage, cv::Size(), 0.5, 0.5);
    cv::imshow("LicensePlates", showImage);
    cv::waitKey(200);

    cv::destroyAllWindows();
}

std::vector<cv::Mat> FrameProcessor::getLicensePlateImages(const shared_ptr<LicensePlate> &licensePlate) {
    vector<cv::Mat> lpImages;
    if (licensePlate->isSquare()) {
        auto combinedImage = combineInOneLineSquareLpPlate(licensePlate->getPlateImage());
        lpImages.push_back(std::move(combinedImage));
    } else {
        lpImages.push_back(licensePlate->getPlateImage());
    }
    return lpImages;
}

std::vector<std::shared_ptr<LicensePlate>>
FrameProcessor::recognize(const std::vector<std::shared_ptr<LicensePlate>> &licensePlates) {
    std::for_each(licensePlates.begin(), licensePlates.end(), [this](std::shared_ptr<LicensePlate> lp) {
        vector<cv::Mat> lpImages = getLicensePlateImages(lp);

        auto stop= 1;

        auto recognizerResult = lpRecognizerNCNN->predict(lpImages);
        auto [licensePlateLabel, probability] = getLicensePlateLabel(recognizerResult, lp->isSquare());

        bool isValid = isValidLicensePlate(licensePlateLabel, probability);

        if (!isValid)
            lp->setLicensePlateLabel("NOT_RECOGNIZED");
        else
            lp->setLicensePlateLabel(std::move(licensePlateLabel));
    });

    return licensePlates;
}

std::pair<std::string, float>
FrameProcessor::getLicensePlateLabel(const vector<std::pair<std::string, float>> &recognizerResult,
                                     bool isSquarePlate) {
    float probability;
    string licensePlateLabel;

    if (isSquarePlate) {
        licensePlateLabel = templateMatching->processSquareLicensePlate(recognizerResult.front().first,
                                                                        recognizerResult.back().first);
        probability = recognizerResult.front().second * recognizerResult.back().second;
    } else {
        licensePlateLabel = recognizerResult.front().first;
        probability = recognizerResult.front().second;
    }
    return make_pair(licensePlateLabel, probability);
}

bool FrameProcessor::isValidLicensePlate(const string &lpLabel, float probability) {
    auto plateCountry = templateMatching->getCountryCode(lpLabel);
    auto isTemplateMatched = plateCountry != Constants::UNIDENTIFIED_COUNTRY;

    return probability > RECOGNIZER_THRESHOLD && isTemplateMatched;
}

cv::Mat FrameProcessor::combineInOneLineSquareLpPlate(const cv::Mat &lpImage) {
    auto blackImage = cv::Mat(Constants::RECT_LP_H, Constants::BLACK_IMG_WIDTH, CV_8UC3,
                              cv::Scalar(0, 0, 0));
    auto topHalf = lpImage(cv::Rect(0, 0, Constants::SQUARE_LP_W, Constants::SQUARE_LP_H / 2));
    auto bottomHalf = lpImage(cv::Rect(0, Constants::SQUARE_LP_H / 2, Constants::SQUARE_LP_W,
                                       Constants::SQUARE_LP_H / 2));

    cv::Mat combinedPlateImage;
    cv::hconcat(topHalf, blackImage, topHalf);
    cv::hconcat(topHalf, bottomHalf, combinedPlateImage);
    return combinedPlateImage;}


