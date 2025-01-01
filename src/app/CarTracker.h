//
// Created by artyk on 4/27/2023.
//

#pragma once

#include <memory>
#include <unordered_map>
#include <utility>

#include "LicensePlate.h"
#include "CalibParams.h"
#include "../client/FrameData.h"
#include "Car.h"
#include "Package.h"
#include "../SharedQueue.h"

class CarTracker : public ILogger {

public:
    explicit CarTracker(std::shared_ptr<CalibParams> calibParams, int platesCount, std::shared_ptr<SharedQueue<std::shared_ptr<Package>>> packageQueue);

    void track(std::shared_ptr<FrameData> &frame, std::vector<std::shared_ptr<LicensePlate>> &untrackedLicensePlates);

    std::vector<std::shared_ptr<Car>> removeOldCars();

    int getSizeOfCurrentCars() const;

    void sentPackages();

private:
    void
    drawAllCurrentCars(const cv::Mat &frame, std::vector<std::shared_ptr<LicensePlate>> &untrackedLicensePlates) const;

    void addNewCar(const std::shared_ptr<Car> &car);

    void increaseCarsAge();

    bool isSameCar(const std::shared_ptr<FrameData> &currentFrame, const std::shared_ptr<LicensePlate> &newLicensePlate,
                   const std::shared_ptr<Car> &car, float distance);

    static float distanceBetweenCars(const std::shared_ptr<Car> &car, const std::shared_ptr<LicensePlate> &newCar);

    bool isCarOld(const std::shared_ptr<Car> &car) const;

    bool isCarNew(const std::shared_ptr<Car> &car) const;

    void resetCarAge(const std::shared_ptr<Car> &car);


    std::unordered_set<std::shared_ptr<Car>> currentCars;
    std::unordered_map<std::shared_ptr<Car>, int> carsAges;
    std::shared_ptr<CalibParams> calibrationParams;
    std::shared_ptr<SharedQueue<std::shared_ptr<Package>>> packagedQueue;

    static const int EDIT_DISTANCE_THRESHOLD = 1;
    const int OLD_AGE_THRESHOLD = 14;
    const float MAX_TIMESTAMP_DIFF_BETWEEN_FRAMES = 0.4;
    int PLATES_COUNT;

    void createAndPushPackage(const std::shared_ptr<LicensePlate> &licensePlate);

    static std::shared_ptr<Package> createPackage(const std::shared_ptr<LicensePlate> &licensePlate);


};

