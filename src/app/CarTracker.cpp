//
// Created by artyk on 4/27/2023.
//

#include "CarTracker.h"

#include <utility>

using namespace std;

CarTracker::CarTracker(std::shared_ptr<CalibParams> calibParams, int platesCount,  std::shared_ptr<SharedQueue<std::shared_ptr<Package>>> packageQueue) : calibrationParams(
        std::move(calibParams)),
                                                                                    ILogger("CarTracker"),
                                                                                    PLATES_COUNT{platesCount}, packagedQueue{std::move(packageQueue)} {
    LOG_INFO("Car Tracker for Camera %s initialized", calibrationParams->getCameraIp().c_str());
}

void CarTracker::track(std::shared_ptr<FrameData> &frame,
                       std::vector<std::shared_ptr<LicensePlate>> &untrackedLicensePlates) {

    increaseCarsAge();

    if (currentCars.empty()) {
        for (auto &lp: untrackedLicensePlates)
            addNewCar(make_shared<Car>(PLATES_COUNT, frame, lp));
        return;
    }

    if (!untrackedLicensePlates.empty()) {
        vector<vector<float>> untrackedToExistingCarDistances;
        vector<vector<float>> existingToUntrackedCarDistances;
        vector<shared_ptr<Car>> carPointers;
        for (int i = 0; i < untrackedLicensePlates.size(); i++) {
            untrackedToExistingCarDistances.emplace_back(vector<float>());
            untrackedToExistingCarDistances.back().reserve(currentCars.size());
        }

        for (int i = 0; i < currentCars.size(); i++) {
            existingToUntrackedCarDistances.emplace_back(vector<float>());
            existingToUntrackedCarDistances.back().reserve(untrackedLicensePlates.size());
        }

        for (auto &car: currentCars) {
            carPointers.emplace_back(shared_ptr<Car>(car));
        }
        for (int untrackedCarIndex = 0; untrackedCarIndex < untrackedLicensePlates.size(); untrackedCarIndex++) {
            auto &newCar = untrackedLicensePlates[untrackedCarIndex];
            for (int carIndex = 0; carIndex < carPointers.size(); carIndex++) {
                auto &car = carPointers[carIndex];
                float distance = distanceBetweenCars(car, newCar);
                untrackedToExistingCarDistances[untrackedCarIndex].push_back(distance);
                existingToUntrackedCarDistances[carIndex].push_back(distance);
            }
        }
        int untrackedCarIndex = 0;
        for (auto &lpToCarsDistances: untrackedToExistingCarDistances) {

            int indexOfClosestCar =
                    min_element(lpToCarsDistances.begin(), lpToCarsDistances.end()) - lpToCarsDistances.begin();

            auto &closestCarToLpsDistances = existingToUntrackedCarDistances[indexOfClosestCar];
            int indexOfClosestLp = min_element(closestCarToLpsDistances.begin(),
                                               closestCarToLpsDistances.end()) - closestCarToLpsDistances.begin();
            auto &closestCar = carPointers[indexOfClosestCar];
            auto &untrackedCar = untrackedLicensePlates[untrackedCarIndex];
            auto distance = lpToCarsDistances[indexOfClosestCar];
            if (untrackedCarIndex == indexOfClosestLp && !isCarNew(closestCar) &&
                isSameCar(frame, untrackedCar, closestCar, distance)) {
                closestCar->addNewData(frame, untrackedCar);
                resetCarAge(closestCar);
            } else {
                addNewCar(make_shared<Car>(PLATES_COUNT, frame, untrackedCar));
            }
            untrackedCarIndex++;
        }
    }

    if (!currentCars.empty()) {
        std::for_each(currentCars.begin(), currentCars.end(), [this](const std::shared_ptr<Car> &currentCar) {
            bool platesCollectedFlag = currentCar->doesPlatesCollected();
            bool isOnMask = calibrationParams->isLicensePlateInSelectedArea(currentCar->getLicensePlateInfos().back(),
                                                                            "sub");
            auto stop =1;
            if (platesCollectedFlag && isOnMask) {
                currentCar->setState(States::haveToSend);
            }
        });
    }
}

std::vector<std::shared_ptr<Car>> CarTracker::removeOldCars() {

    vector<shared_ptr<Car>> oldCars;

    for (auto carPtr = currentCars.begin(); carPtr != currentCars.end();) {

        auto &currentCar = *carPtr;

        if (isCarOld(currentCar)) {
            carsAges.erase(currentCar);
            oldCars.emplace_back(currentCar);
            carPtr = currentCars.erase(carPtr);
        } else {
            carPtr++;
        }
    }

    return std::move(oldCars);
}

void CarTracker::addNewCar(const shared_ptr<Car> &car) {
    currentCars.emplace(car);
    carsAges.emplace(car, 0);
}

void CarTracker::increaseCarsAge() {
    for (auto &[_, age]: carsAges) {
        age++;
    }
}

bool CarTracker::isSameCar(const shared_ptr<FrameData> &currentFrame, const shared_ptr<LicensePlate> &newLicensePlate,
                           const shared_ptr<Car> &car, float distance) {
    if (newLicensePlate->getPlateLabel() == car->getLicensePlateInfos().back()->getPlateLabel()) {
        return true;
    }

    int distanceBetweenLPs = Utils::calculateEditDistance(newLicensePlate->getPlateLabel(),
                                                          car->getLicensePlateInfos().back()->getPlateLabel());
    if (distanceBetweenLPs <= EDIT_DISTANCE_THRESHOLD)
        return true;

    if (abs(currentFrame->getRTPtimestamp() - car->getCarFrames().back()->getRTPtimestamp()) >
        MAX_TIMESTAMP_DIFF_BETWEEN_FRAMES)
        return false;
    return false;
}

float CarTracker::distanceBetweenCars(const shared_ptr<Car> &car, const shared_ptr<LicensePlate> &newCar) {
    return cv::norm(
            car->getLicensePlateInfos().back()->getCenter() - newCar->getCenter()
    );
}

bool CarTracker::isCarOld(const shared_ptr<Car> &car) const {
    return carsAges.find(car) != carsAges.end() && carsAges.at(car) > OLD_AGE_THRESHOLD ||
           car->getState() == States::sent;
}

bool CarTracker::isCarNew(const shared_ptr<Car> &car) const {
    return carsAges.find(car) != carsAges.end() && carsAges.at(car) == 0;
}

void CarTracker::resetCarAge(const shared_ptr<Car> &car) {
    carsAges.at(car) = 0;
}

int CarTracker::getSizeOfCurrentCars() const {
    return currentCars.size();
}


void CarTracker::drawAllCurrentCars(const cv::Mat &frame,
                                    std::vector<std::shared_ptr<LicensePlate>> &untrackedLicensePlates) const {

    cv::Mat cloneFrame = frame.clone();

    for (const auto &lp: untrackedLicensePlates)
        circle(cloneFrame, lp->getCenter(), 1, cv::Scalar(255, 0, 0), 5, 8, 0);

    for (const auto &car: currentCars)
        circle(frame, car->getLicensePlateInfos().front()->getCenter(), 1, cv::Scalar(0, 255, 0), 5, 8, 0);


    cv::imshow("CurrentCars", frame);
    cv::imshow("CurrentLicensePlates", cloneFrame);

    cv::waitKey(0);
    cv::destroyAllWindows();


}

void CarTracker::sentPackages() {
    if (!currentCars.empty()) {
        std::for_each(currentCars.begin(), currentCars.end(), [this](const std::shared_ptr<Car> &currentCar) {
            if (currentCar->getState() == States::haveToSend) {
                auto sto = 1;

                currentCar->setState(States::sent);
                auto licensePlate = currentCar->getMostCommonLicensePlate();
                LOG_INFO("Found plate for sending %s", licensePlate->getPlateLabel().c_str());
                createAndPushPackage(licensePlate);
            }else if(currentCar->getState() == States::initial && currentCar->doesPlatesCollected()){
                auto sto = 1;

                currentCar->setState(States::sent);
                auto licensePlate = currentCar->getMostCommonLicensePlate();
                LOG_INFO("Found plate for sending but was not set its state %s", licensePlate->getPlateLabel().c_str());
                createAndPushPackage(licensePlate);
            }
        });
    }
}

std::shared_ptr<Package> CarTracker::createPackage(const shared_ptr<LicensePlate> &licensePlate) {
    auto package = make_shared<Package>(licensePlate->getCameraIp(), licensePlate->getPlateLabel(),
                                        licensePlate->getCarImage());
    return package;
}

void CarTracker::createAndPushPackage(const shared_ptr<LicensePlate> &licensePlate) {
    auto package = createPackage(licensePlate);
    packagedQueue->push(std::move(package));
}
