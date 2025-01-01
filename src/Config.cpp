#include "Config.h"

using namespace std;
using json = nlohmann::json;

vector<string> cameraIps;
string resultSendIp;
string nodeIp;

bool useDecodeGpu = false;
int platesCount = 5;
int timeBetweenLPSend = 3;
float recognizerThreshold = 0.85;
float detectorThreshold = 0.75;

const vector<string> &Config::getCameraIps() {
    return cameraIps;
}

const string &Config::getResultSendIp() {
    return resultSendIp;
}

const string &Config::getNodeIp() {
    return nodeIp;
}

bool Config::useGPUDecode() {
    return useDecodeGpu;
}

const int &Config::getPlatesCount() {
    return platesCount;
}

const int &Config::getTimeSentPlates() {
    return timeBetweenLPSend;
}

const float &Config::getRecognizerThreshold() {
    return recognizerThreshold;
}

const float &Config::getDetectorThreshold() {
    return detectorThreshold;
}

bool Config::parseJson(const string &fileName) {
    try {
        ifstream configFile(fileName);
        if (!configFile.is_open())
            throw runtime_error("Config file not found");

        json configs = json::parse(configFile);

        cameraIps = Utils::splitString(configs["camera_ips"].get<string>(), ",");
        resultSendIp = configs["result_send_ip"].get<string>();
        nodeIp = configs["node_ip"].get<string>();

        if (configs.find("plates_count") == configs.end())
            platesCount = 5;
        else platesCount = configs["plates_count"].get<int>();

        if (configs.find("time_between_sent_plates") == configs.end())
            timeBetweenLPSend = 3;
        else timeBetweenLPSend = configs["time_between_sent_plates"].get<int>();

        if (configs.find("use_gpu_decode") == configs.end() || configs["use_gpu_decode"].get<int>() == 1)
            useDecodeGpu = true;
        else
            useDecodeGpu = false;

        if (configs.find("rec_threshold") == configs.end())
            recognizerThreshold = 0.85;
        else recognizerThreshold = configs["rec_threshold"].get<float>();

        if (configs.find("det_threshold") == configs.end())
            detectorThreshold = 0.75;
        else detectorThreshold = configs["det_threshold"].get<float>();

    } catch (exception &e) {
        cout << "Exception occurred during config parse: " << e.what() << endl;
        return false;
    }
    return true;
}












