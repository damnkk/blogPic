#ifndef MYWINDOWMANAGER_H
#define MYWINDOWMANAGER_H
#include <cinder/cinder.h>
#include <cinder/app/App.h>
#include <cinder/CinderImGui.h>
#include <vector>
class WindowManager{
public:
    void init(ci::app::App* app);
    enum WindowState{
        INVALID = -1,
        HIDDEN = 0,
        SHOWN = 1
    };

    struct UIWindow{
        UIWindow() = default;
        UIWindow(std::function<void(UIWindow*,std::vector<void*>)> renderFunc):_onGUI(renderFunc){
        }

        void addSubWindow(std::string name, std::function<void(UIWindow*,std::vector<void*>)> renderFunc);
        WindowState getWindowState(int id);
        void setWindowState(int id, WindowState state);
        std::function<void(UIWindow*,std::vector<void*>)> _onGUI;
        WindowManager* _manager;
        std::unordered_map<std::string,int> _subWindow;
    };
    void onGUI();

private:
    int allocWindow(std::function<void(UIWindow*,std::vector<void*>)> renderFunc);
    void destroyWindow(int id);
    friend struct UIWindow;
    std::vector<std::shared_ptr<UIWindow>> _subWindows;
    std::vector<WindowState> _subWindowState;
    std::vector<std::shared_ptr<UIWindow>> _mainWindow;
    std::vector<int> _windowUsage;
    std::atomic_int _windowsCounter=0;
    ci::app::App* _app;
};

#endif // MYWINDOWMANAGER_H