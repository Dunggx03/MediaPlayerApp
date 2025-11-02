#ifndef USB_UTILS_H
#define USB_UTILS_H

#include <string>

class USBUtils {
public:
    USBUtils() = default;
    ~USBUtils() = default;


    bool isDeviceRemovable(const std::string& deviceName);
    std::string detectUSBMount();
    bool unmountUSB(const std::string& mountPath);
    std::string getRootDevice();

    
};
#endif
