#include "Package.h"

using namespace std;
using json = nlohmann::json;

Package::Package(string cameraIp, string licensePlateLabel,
                 cv::Mat carImage)
        : cameraIp(std::move(cameraIp)), licensePlateLabel(std::move(licensePlateLabel)),
          carImage(std::move(carImage)) {
};



string Package::getPackageJsonString() const {
    json packageJson;
    packageJson["ip_address"] = cameraIp;
    packageJson["car_number"] = licensePlateLabel;
    packageJson["car_picture"] = Utils::encodeImgToBase64(carImage);

    return packageJson.dump();
}

const string &Package::getPlateLabel() const {
    return licensePlateLabel;
}

const string &Package::getCameraIp() const {
    return cameraIp;
}
