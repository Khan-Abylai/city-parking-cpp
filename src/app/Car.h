#pragma once

#include <unordered_set>

#include "LicensePlate.h"
#include "Utils.h"
#include "Counter.h"
#include "../ILogger.h"
#include "../client/FrameData.h"
#include "CalibParams.h"

enum class States {
    initial = 0,
    haveToSend = 1,
    sent = 2
};

struct CounterHashFunction {
    struct Hash {
        std::size_t operator()(std::shared_ptr<Counter<std::shared_ptr<LicensePlate>>> const &p) const noexcept {
            return std::hash<std::string>{}(p->getItem()->getPlateLabel());
        }
    };

    struct Compare {
        size_t operator()(std::shared_ptr<Counter<std::shared_ptr<LicensePlate>>> const &a,
                          std::shared_ptr<Counter<std::shared_ptr<LicensePlate>>> const &b) const {
            return a->getItem()->getPlateLabel() == b->getItem()->getPlateLabel();
        }
    };
};

class Car : public ILogger {
public:
    Car(const int &platesCount, std::shared_ptr<FrameData> carFrame, std::shared_ptr<LicensePlate> licensePlate);

    void addLicensePlateToCount(std::shared_ptr<LicensePlate> licensePlate);

    const std::shared_ptr<LicensePlate> &getMostCommonLicensePlate() const;

    bool doesPlatesCollected();

    std::vector<std::shared_ptr<FrameData>> getCarFrames() const;

    std::vector<std::shared_ptr<LicensePlate>> getLicensePlateInfos() const;

    void addNewData(std::shared_ptr<FrameData> carFrame,
                    std::shared_ptr<LicensePlate> carInfo);

    States getState();

    void setState(States state);

private:
    States STATE = States::initial;
    int MIN_NUM_PLATES_COLLECTED;

    std::vector<std::shared_ptr<FrameData>> carFrames;
    std::vector<std::shared_ptr<LicensePlate>> licensePlateInfos;

    std::unordered_set<std::shared_ptr<Counter<std::shared_ptr<LicensePlate>>>,
            CounterHashFunction::Hash, CounterHashFunction::Compare> licensePlates;

    std::shared_ptr<Counter<std::shared_ptr<LicensePlate>>> mostCommonPlate;
    std::vector<cv::Point2f> trackingPoints;

};
