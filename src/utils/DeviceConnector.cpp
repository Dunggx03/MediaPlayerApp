#include "utils/DeviceConnector.h"
#include <iostream>

DeviceConnector::DeviceConnector() {
    std::cout << "DeviceConnector: Skeleton created." << std::endl;
}

DeviceConnector::~DeviceConnector() {
}

bool DeviceConnector::connect(const std::string& port) {
    std::cout << "DeviceConnector: (Skeleton) connect to " << port << std::endl;
    // to open UART in here
    return true;
}

void DeviceConnector::sendData(const std::string& data) {
    std::cout << "DeviceConnector: (Skeleton) send: " << data << std::endl;
    // write into file 
}

std::string DeviceConnector::readData() {
    // read from file
    return ""; // no data 
}