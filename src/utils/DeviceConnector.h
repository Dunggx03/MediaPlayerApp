#pragma once
#include <string>

class DeviceConnector {
public:
    DeviceConnector();
    ~DeviceConnector();
    bool connect(const std::string& port);

    void sendData(const std::string& data);

    std::string readData();
};