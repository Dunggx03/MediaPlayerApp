#pragma once
#include <memory>

class UIManager;
class NcursesUI;
class AppController;

class App {
public:
    App();
    ~App();

    bool init();
    void run();

private:
    std::unique_ptr<NcursesUI> ui;
    std::unique_ptr<UIManager> uiManager;
    std::unique_ptr<AppController> appController;
};