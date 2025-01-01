#include "Car.h"

using namespace std;

Car::Car(const int &platesCount, std::shared_ptr<FrameData> carFrame, std::shared_ptr<LicensePlate> licensePlate) : MIN_NUM_PLATES_COLLECTED{platesCount} {
    carFrames.emplace_back(std::move(carFrame));
    licensePlateInfos.emplace_back(licensePlate);
    this->addLicensePlateToCount(std::move(licensePlate));
}

void Car::addLicensePlateToCount(shared_ptr<LicensePlate> licensePlate) {
    auto newLicensePlate = make_shared<Counter<shared_ptr<LicensePlate>>>(std::move(licensePlate));
    auto licensePlateIterator = licensePlates.find(newLicensePlate);
    if (licensePlateIterator != licensePlates.end()) {
        auto foundLicensePlate = *licensePlateIterator;
        foundLicensePlate->incrementOccurrence();
        foundLicensePlate->renewItem(newLicensePlate->getItem());

        if (!mostCommonPlate || mostCommonPlate->getNumberOfOccurrence() < foundLicensePlate->getNumberOfOccurrence())
            mostCommonPlate = foundLicensePlate;
    } else {
        if (!mostCommonPlate)
            mostCommonPlate = newLicensePlate;
        licensePlates.insert({newLicensePlate});
    }
}

const shared_ptr<LicensePlate> &Car::getMostCommonLicensePlate() const {

    LOG_INFO("---- found plates -----");
    for (const auto &plate: licensePlates) {
        LOG_INFO("plate: %s, count: %d", plate->getItem()->getPlateLabel().data(), plate->getNumberOfOccurrence());
    }
    return mostCommonPlate->getItem();
}


bool Car::doesPlatesCollected() {
    int platesCollected = 0;
    for (const auto &plate: licensePlates) platesCollected += plate->getNumberOfOccurrence();

    if (platesCollected >= MIN_NUM_PLATES_COLLECTED) return true;
    return false;
}

std::vector<std::shared_ptr<FrameData>> Car::getCarFrames() const {
    return carFrames;
}

std::vector<std::shared_ptr<LicensePlate>> Car::getLicensePlateInfos() const {
    return licensePlateInfos;
}


void Car::addNewData(std::shared_ptr<FrameData> carFrame, std::shared_ptr<LicensePlate> carInfo) {
    carFrames.emplace_back(std::move(carFrame));
    licensePlateInfos.emplace_back(carInfo);
    this->addLicensePlateToCount(std::move(carInfo));
}

States Car::getState() {
    return STATE;
}

void Car::setState(States state) {
    this->STATE = state;
}

