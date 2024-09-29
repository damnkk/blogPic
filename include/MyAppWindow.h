#ifndef MYAPPWINDOW_H
#include <imgui.h>
#include <string>
#include <vector>
#include <filesystem>
#include <cinder/app/App.h>
struct MyApp;
class MyAppWindowBase{
public:
    virtual void Draw() = 0;
protected:
    ImVec2 windowPos;
    ImVec2 windowSize;

};

class FileBroserWindow: public MyAppWindowBase{
public:
   
    FileBroserWindow(MyApp*app){_app = app;};
    ~FileBroserWindow(){};    
    void setRootPaths(const std::vector<std::string>& paths){_rootPaths = paths;};
    std::vector<std::string> getRootPaths(){return _rootPaths;}
    void Draw() override;
    void showFileBrowser(const std::filesystem::path& path);
    bool _needDraw = false;
protected:
    std::string _clickedPath="";
    std::vector<std::string> _rootPaths;
    MyApp* _app;

};

#endif // MYAPPWINDOW_H