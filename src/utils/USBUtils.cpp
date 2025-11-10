#include "USBUtils.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

bool USBUtils::isDeviceRemovable(const std::string& deviceName) {
    std::string sysPath = "/sys/block/" + deviceName + "/removable";
    std::ifstream file(sysPath);
    if (!file.is_open()) return false;

    int flag = 0;
    file >> flag;
    file.close();
    return (flag == 1);
}

std::string USBUtils::getRootDevice() {
    FILE* pipe = popen("findmnt -no SOURCE /", "r");
    if (!pipe) return "";
    char buffer[128];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe)) result += buffer;
    pclose(pipe);
    result.erase(std::remove_if(result.begin(), result.end(), ::isspace), result.end());
    return result;
}

std::string USBUtils::detectUSBMount() {
    const char* username = getenv("USER");
    if (!username) {
        std::cerr << "[USBUtils] Cannot detect USER environment.\n";
        return "";
    }

    std::string rootDev = getRootDevice();
    std::string rootDisk = "";
    if (!rootDev.empty()) {
        std::string rootBase = fs::path(rootDev).filename().string(); 
        rootDisk = rootBase.substr(0, 3); 
        std::cout << "[USBUtils]  Root device: " << rootDev 
                  << " (Disk: " << rootDisk << ")\n";
    } else {
        std::cerr << "[USBUtils]Cannot determine root device.\n";
    }

    std::vector<std::string> basePaths = {
        "/media/" + std::string(username),
        "/run/media/" + std::string(username)
    };

    for (const auto& base : basePaths) {
        if (!fs::exists(base)) continue;

        for (const auto& entry : fs::directory_iterator(base)) {
            if (!fs::is_directory(entry.path())) continue;

            std::string label = entry.path().filename().string();
            std::string devLink = "/dev/disk/by-label/" + label;

            if (!fs::exists(devLink)) {
                std::cout << "[USBUtils]  Skipping non-device entry: " << label << "\n";
                continue;
            }

            std::string devTarget;
            try {
                devTarget = fs::read_symlink(devLink).filename().string(); 
            } catch (...) {
                std::cerr << "[USBUtils]  Failed to resolve symlink for " << devLink << "\n";
                continue;
            }

            std::string deviceName = devTarget.substr(0, 3); // sda, sdb, ...

            
            if (!rootDisk.empty() && deviceName == rootDisk) {
                std::cout << "[USBUtils]  Skipping system disk: " << deviceName << "\n";
                continue;
            }

            if (isDeviceRemovable(deviceName)) {
                std::cout << "[USBUtils]  Found removable USB: " << entry.path() << "\n";
                return entry.path().string();
            } else {
                std::cout << "[USBUtils]  Ignored non-removable device: " << deviceName << "\n";
            }
        }
    }

    std::cerr << "[USBUtils]  No USB device found.\n";
    return "";
}

bool USBUtils::unmountUSB(const std::string& mountPath) {
    if (mountPath.empty() || !fs::exists(mountPath)) {
        std::cerr << "[USBUtils]  Invalid mount path for unmount: " << mountPath << "\n";
        return false;
    }

    std::string cmd = "sudo -n umount " + mountPath + " >/dev/null 2>&1";
    int ret = std::system(cmd.c_str());
    if (ret == 0) {
        std::cout << "[USBUtils]  USB safely unmounted: " << mountPath << "\n";
        return true;
    } else {
        std::cerr << "[USBUtils]  Failed to unmount: " << mountPath << "\n";
        return false;
    }
}
