#include "app/App.h"
#include <iostream>
#include <fstream>
#include <clocale>

int main() {
    // --- SET LOCALE FIRST ---
    if (setlocale(LC_ALL, "") == NULL) {
         std::cerr << "Warning: Could not set locale." << std::endl;
    }
    
    // --- REDIRECT OUTPUT TO FILE ---
    std::ofstream log_file("debug.log");
    if (!log_file.is_open()) {
        std::cerr << "FATAL: Could not open debug.log for writing!" << std::endl;
        return 1;
    }
    
    auto cout_buf = std::cout.rdbuf();
    auto cerr_buf = std::cerr.rdbuf();

    
    std::cout.rdbuf(log_file.rdbuf());
    std::cerr.rdbuf(log_file.rdbuf());



    std::cout << "--- Application Started ---" << std::endl;
{
    App myApp;

    if (!myApp.init()) {
        std::cerr << "Application failed to initialize." << std::endl;
        
        std::cout.rdbuf(cout_buf);
        std::cerr.rdbuf(cerr_buf);
        log_file.close();
        std::cerr << "FATAL: App initialization failed. Check debug.log for details." << std::endl;
        return 1;
    }

    myApp.run();

    std::cout << "--- Application Shutting Down ---" << std::endl;
}
    std::cout.rdbuf(cout_buf);
    std::cerr.rdbuf(cerr_buf);
    log_file.close();

    std::cout << "Application finished. Check debug.log for details." << std::endl;
    return 0;
}