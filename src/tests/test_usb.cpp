#include <iostream>
#include <cstdio>
#include <string>
#include <algorithm>

std::string detectWSLUSBDrive() {
    const char* cmd =
    "/mnt/c/Windows/System32/WindowsPowerShell/v1.0/powershell.exe "
    "-Command \"(Get-Volume | Where-Object {\\$_.DriveType -eq 'Removable'} | Select -ExpandProperty DriveLetter)\"";


    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        std::cerr << "[USBUtils] ❌ Cannot open PowerShell pipe\n";
        return "";
    }

    char buffer[128];
    std::string result;

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);

    // Xóa BOM UTF-8 nếu có (0xEF,0xBB,0xBF)
    if (!result.empty() && (unsigned char)result[0] == 0xEF) {
        result.erase(0, 3);
    }

    // Xóa whitespace thừa: \n \r \t space
    result.erase(std::remove_if(result.begin(), result.end(), ::isspace), result.end());

    if (result.empty()) {
        std::cerr << "[USBUtils] ⚠️ PowerShell returned no drive letter\n";
        return "";
    }

    return result + ":"; 
}

bool mountWSLDrive(const std::string& driveLetter, const std::string& mountPath) {
    std::string cmd = "sudo -n mount -t drvfs " + driveLetter + " " + mountPath + " >/dev/null 2>&1";
    int ret = std::system(cmd.c_str());
    if (ret == 0) {
        std::cout << "[USBUtils] ✅ Mounted " << driveLetter << " to " << mountPath << "\n";
        return true;
    } else {
        std::cerr << "[USBUtils] ⚠️ Mount command failed: " << cmd << "\n";
        return false;
    }
}

bool reloadUSBLibrary(const std::string& mountPath) {
    std::string newDrive = detectWSLUSBDrive();
    if (newDrive.empty()) {
        std::cerr << "[USBUtils] ❌ Cannot reload. No removable USB found.\n";
        return false;
    }

    std::cout << "[USBUtils] 🔁 USB reload: detected drive " << newDrive << "\n";

    // (1) thử umount trước nếu đang mount
    std::string cmd = "sudo umount " + mountPath + " >/dev/null 2>&1";
    std::system(cmd.c_str());

    // (2) mount lại
    if (!mountWSLDrive(newDrive, mountPath)) {
        std::cerr << "[USBUtils] ❌ Reload failed: cannot mount " << newDrive << "\n";
        return false;
    }

    std::cout << "[USBUtils] ✅ Reload successfully. USB re-mounted at " << mountPath << "\n";
    return true;
}


bool ejectUSB(const std::string& mountPath) {
    std::string cmd = "sudo -n umount " + mountPath + " >/dev/null 2>&1";
    int ret = std::system(cmd.c_str());
    if (ret == 0) {
        std::cout << "[USBUtils] ✅ USB unmounted (ejected) successfully from: " << mountPath << "\n";
        return true;
    } else {
        std::cerr << "[USBUtils] ❌ Failed to unmount USB at: " << mountPath << "\n";
        return false;
    }
}


int main() {
    std::string drive = detectWSLUSBDrive();
    if (drive.empty()) {
        std::cout << "❌ No USB detected\n";
    } else {
        std::cout << "✅ USB detected: " << drive << "\n";
    }
std::string mountPath = "/home/quynhmai/mock/MediaPlayer/usb";
    // ✅ Dò đúng removable USB drive bằng PowerShell (không nhầm với HDD/SSD khác)
    std::string detectedDrive = detectWSLUSBDrive();
    if (detectedDrive.empty()) {
        std::cerr << "[USBUtils] ⚠️ No removable USB drive detected in WSL.\n";
    }

    std::cout << "[USBUtils] ✅ USB drive detected: " << detectedDrive << "\n";

    // Mount vào thư mục usb/
    if (!mountWSLDrive(detectedDrive, mountPath)) {
        std::cerr << "[USBUtils] ❌ Failed to mount " << detectedDrive << "\n";
    }

    std::cout << "[USBUtils] ✅ USB mounted successfully at: " << mountPath << "\n";

    while (true) {
    std::cout << "\n===== USB CONTROL MENU =====\n";
    std::cout << "1. Eject USB\n";
    std::cout << "2. Reload USB\n";
    std::cout << "0. Exit\n";
    std::cout << "Chọn: ";

    int choice;
    std::cin >> choice;

    if (choice == 1) {
        if(ejectUSB(mountPath)){
            std::cout<<"eject successfully"<<std::endl;
        }
    } 
    else if (choice == 2) {
        if(reloadUSBLibrary(mountPath)){
            std::cout<<"reload successfully"<<std::endl;
        }
    }
    else if (choice == 0) {
        std::cout << "Bye!\n";
        break;
    } 
    else {
        std::cout << "⚠️ Lựa chọn không hợp lệ\n";
    }
    }
    return 0;
}