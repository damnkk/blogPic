#include <MyApp.h>
#if WIN32
#include <Windows.h>
#endif
#include <filesystem>
#include <MyAppWindow.h>
#include <MyProject.h>
#include <chrono>

std::string builtIntVertShader =R"(
#version 460  
layout (location = 0) in vec4   ciPosition;
layout (location = 1) in vec2   ciTexCoord0;
out vec2 uv;
void main(){   
    uv = vec2(ciPosition.xy)+0.5;
    gl_Position = vec4(ciPosition.x*2.0, ciPosition.y*2.0, ciPosition.z, 1.0);
})";
std::string builtIntFragShader =R"(
#version 460 
out vec4 FragColor;
in vec2 uv;
void main(){   
    FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
)";


void MyApp::keyDown(cinder::app::KeyEvent event){
}

void MyApp::draw(){
    glClearColor(0.2,0.0,0.2,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    if(_myProject&& _myProject->type==MyProject::ProjectType::ShaderToy){
        auto renderAble =(ScreenRenderable*) (_myScene->_rootNode->_components.front().get());
        renderAble->batch->draw();
    }
}

void MyApp::update(){
    ImGui::BeginMainMenuBar();
    if(ImGui::BeginMenu("File")){
        if(ImGui::MenuItem("New Project")){
            _popupState = PopupState::NewProj;
        }
        if(ImGui::MenuItem("Save")){
            
        }
        if(ImGui::MenuItem("Exit")){
            this->quit();
        }
        ImGui::EndMenu();
    }
    if(ImGui::BeginMenu("Settings")){
        if(ImGui::MenuItem("Full Screen")){
            if(!this->getWindow()->isFullScreen())
                this->getWindow()->setFullScreen(true);
            else 
                this->getWindow()->setFullScreen(false);
        }
#if WIN32
        if(ImGui::MenuItem("Wall Paper Mode")){
            auto window = (HWND)this->getWindow()->getNative();
            LONG_PTR style = GetWindowLongPtr(window, GWL_EXSTYLE);
            style |= WS_EX_LAYERED | WS_EX_TOOLWINDOW;
            SetWindowLongPtr(window, GWL_EXSTYLE, style);
            SetLayeredWindowAttributes(window, RGB(0, 0, 0), 255, LWA_COLORKEY);
            SetWindowPos(window, HWND_BOTTOM, 0, 0, 800, 600, SWP_NOACTIVATE | SWP_NOZORDER);
        }
#endif
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
    switch (_popupState) {
        case MyApp::PopupState::NewProj:{
            std::string basePath = "";
#if WIN32
            std::vector<std::string> rootPaths;
            DWORD drives = GetLogicalDrives();
            if(drives != 0){
                for (char drive = 'A'; drive <= 'Z'; ++drive) {
                    if (drives & (1 << (drive - 'A'))) { // 检查该驱动器是否可用
                        std::string rootPath = std::string(1, drive) + ":\\";
                        rootPaths.push_back(rootPath);
                    }
                }
            }
            if(!rootPaths.empty()){
                _fileBroserWindow->setRootPaths(rootPaths);
                _fileBroserWindow->Draw();
            }
#endif
            break;
        }
        case MyApp::PopupState::WhatEver:
            break;
        case MyApp::PopupState::None:
            break;
    }
    checkUpdateShaders();

}

inline bool isShaderNeedUpdate(std::string& path,std::unordered_map<std::string, std::filesystem::file_time_type>& maps){
    if(!std::filesystem::exists(path)){
        return false;
    }
    auto shaderFind = maps.find(path.substr(path.find_last_of("/\\"),path.size()));
    if(shaderFind==maps.end()||shaderFind->second!=std::filesystem::last_write_time(path)){
        return true;
    }else{
        return false;
    }
}

struct ExplosionEvent:public ecs::BaseEvent{
    ExplosionEvent():_explosionRadius(0.0){}
    ExplosionEvent(float radius):_explosionRadius(radius){}
    float _explosionRadius;

};

struct ExplosionSystem:public ecs::Receiver<ExplosionSystem>{
    ExplosionSystem(){};
    ~ExplosionSystem(){};

    void receive(ecs::BaseEvent& event){
        ExplosionEvent* explosionEvent = dynamic_cast<ExplosionEvent*>(&event);
        if(explosionEvent){
            std::cout<<"Boom!! a Bomb exploded with radius :"<< explosionEvent->_explosionRadius<<std::endl;
        }
    }

    void receive(ecs::EntityCreatedEvent& event){
        std::cout<<"Entity Created"<<std::endl;
    }
    void receive(ecs::EntityDestroyedEvent& event){
        std::cout<<"Entity Destroyed"<<std::endl;
    }
    template<typename C>
    void receive(ecs::ComponentAddedEvent<C>& event){
        std::cout<<"Component"<<typeid(event).name()<<" Added"<<std::endl;
    }
    template<typename C>
    void receive(ecs::ComponentRemovedEvent<C>& event){
        std::cout<<"Component"<<typeid(event).name()<<" Removed"<<std::endl;
    }
};

struct FuckPussyComponent{
    FuckPussyComponent()=default;
    float beet = 0.0;
};

void MyApp::setup(){
    ImGui::Initialize();
    auto window = this->getWindow();
    auto io = ImGui::GetIO();
    // auto font = io.Fonts->AddFontFromFileTTF("./asset/ttf/QingNiaoHuaGuangJianMeiHei-2.ttf", 18.0f,NULL,io.Fonts->GetGlyphRangesChineseFull());
    adjustForDPI();
    _fileBroserWindow = std::make_shared<FileBroserWindow>(this);

    ecs::EventManager EVTManager;
    ExplosionEvent explosionEvent(23.0);
    ExplosionSystem explosionSystem;
    EVTManager.subscribe<ExplosionEvent>(explosionSystem);
    EVTManager.emit<ExplosionEvent>(explosionEvent);

    ecs::EntityManager EntityMane(EVTManager);
    EVTManager.subscribe<ecs::EntityCreatedEvent>(explosionSystem);
    EVTManager.subscribe<ecs::EntityDestroyedEvent>(explosionSystem);
    EVTManager.subscribe<ecs::ComponentAddedEvent<FuckPussyComponent>>(explosionSystem);
    EVTManager.subscribe<ecs::ComponentRemovedEvent<FuckPussyComponent>>(explosionSystem);
    auto entity = EntityMane.create();
    entity.assign<FuckPussyComponent>();





}

void MyApp::adjustForDPI(){
    float baseRenderingDPI = sqrt(1920*1920+1080*1080)/24.0;
    float deviceDPI = baseRenderingDPI;
#if WIN32
    DISPLAY_DEVICE dd;
    dd.cb = sizeof(DISPLAY_DEVICE);
    if(EnumDisplayDevices(NULL,0,&dd,0)){
        DEVMODE dm;
        dm.dmSize = sizeof(dm);
        if(EnumDisplaySettings(dd.DeviceName,ENUM_CURRENT_SETTINGS,&dm)){
            float screenWidth = float(dm.dmPelsWidth);
            float screenHeight = float(dm.dmPelsHeight);
            HDC hdc = GetDC(NULL);
            float dpix =screenWidth/ GetDeviceCaps(hdc, LOGPIXELSX);
            float dpiy =screenHeight/ GetDeviceCaps(hdc, LOGPIXELSY);
            ReleaseDC(NULL, hdc);
            deviceDPI = sqrt(screenWidth*screenWidth  + screenHeight*screenHeight)/sqrt(dpix*dpix + dpiy*dpiy);
        }
    }
#endif
    float uiScale = deviceDPI/baseRenderingDPI;
    ImGui::GetStyle().ScaleAllSizes(uiScale);
    ImGuiIO&io = ImGui::GetIO();
    io.FontGlobalScale = uiScale;
}

void MyApp::createProject(std::string basePath){
    std::filesystem::create_directory(basePath+"/"+"NewProject");
    basePath += "/NewProject";
    std::string shadersPath = basePath+"/shaders";
    std::filesystem::create_directory(shadersPath);
    std::ofstream file(shadersPath+"/new_project.vert");
    if(file){
        file<<builtIntVertShader;
    }
    file.close();
    file = std::ofstream(shadersPath+"/new_project.frag");
    if(file){
        file<<builtIntFragShader;
    }
    file.close();

    cinder::Json projJson;
    projJson["name"] = "New Project";
    projJson["ProjectType"] = MyProject::ProjectType::ShaderToy;
    projJson["shaders"] = {{"new_project",{{"VertexShader","shaders/new_project.vert"},{"FragmentShader", "shaders/new_project.frag"}}}};
    projJson["basePath"] = basePath;
    file = std::ofstream(basePath+"/new_project.json");
    if(file){
        file<<projJson;
    }
    file.close();
    this->loadProject(projJson);
}

void MyApp::loadProject(const cinder::Json& projJson){
    _myProject = std::make_shared<MyProject>();
    MyProject::ProjectType projectType = projJson["ProjectType"];
    switch (projectType) {
        case MyProject::ProjectType::ShaderToy:
            load2DProject(projJson);
            break;
        default:
            break;
    }
    _myProject->isLoaded = true;
}

void MyApp::load2DProject(const cinder::Json& projJson){
    _myScene = std::make_shared<MyScene>();
    _myScene->_rootNode = std::make_shared<SceneNode>();
    std::shared_ptr<ScreenRenderable> screenRenderable = std::make_shared<ScreenRenderable>();
    addAssetDirectory(projJson["basePath"]);
    auto prog = cinder::gl::GlslProg::create(cinder::app::loadAsset(std::string(projJson["shaders"].front()["VertexShader"])),
                                                        cinder::app::loadAsset(std::string(projJson["shaders"].front()["FragmentShader"])));
    cinder::gl::VboMeshRef quadRef= cinder::gl::VboMesh::create(cinder::geom::Rect());
    screenRenderable->batch = cinder::gl::Batch::create(quadRef,prog);
    _myScene->_rootNode->_components.push_back(screenRenderable);
    _myProject->basePath = projJson["basePath"];
    _myProject->isLoaded = true;
    _myProject->shadersMap["new_project"]= prog;
    _popupState = PopupState::None;
}

void MyApp::checkUpdateShaders(){
     static std::unordered_map<std::string, std::filesystem::file_time_type> lastModifiedTimeStampsMap;
    if(_myProject&&_myProject->isLoaded){
        std::string shadersBasePath = _myProject->basePath+"/shaders";
        for(auto& prog:_myProject->shadersMap){
            std::string vertShaderPath = shadersBasePath+"/"+prog.first+".vert";
            std::string fragShaderPath = shadersBasePath+"/"+prog.first+".frag";
            bool vertNeedUpdate = isShaderNeedUpdate(vertShaderPath,lastModifiedTimeStampsMap);
            bool fragNeedUpdate = isShaderNeedUpdate(fragShaderPath,lastModifiedTimeStampsMap);
            if(vertNeedUpdate||fragNeedUpdate){
                auto shaderProgRef = cinder::gl::GlslProg::create(cinder::app::loadAsset(vertShaderPath),cinder::app::loadAsset(fragShaderPath));
                _myProject->shadersMap[prog.first] = shaderProgRef;
                lastModifiedTimeStampsMap[vertShaderPath.substr(vertShaderPath.find_last_of("/\\"))] = std::filesystem::last_write_time(vertShaderPath);
                lastModifiedTimeStampsMap[fragShaderPath.substr(fragShaderPath.find_last_of("/\\"))] = std::filesystem::last_write_time(fragShaderPath);
                ((ScreenRenderable*)(_myScene->_rootNode->_components.front().get()))->batch ->replaceGlslProg(shaderProgRef);
            }
        }
    }
}