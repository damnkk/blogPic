#include <MyApp.h>
#if WIN32
#include <Windows.h>
#endif

#include <MyBuiltInShaderCode.h>
#include <MyProject.h>
#include <UI/MyAppWindow.h>
#include <rttr/registration.h>

#include <chrono>
#include <filesystem>

void MyApp::keyDown(cinder::app::KeyEvent event){
}

void MyApp::draw() {
  _uiSystem->Draw();
  glClearColor(0.2, 0.0, 0.2, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  if (_myProject && _myProject->type == MyProject::ProjectType::Proj2D) {
    auto renderable = (_myScene->_rootNode->_entity.component<ScreenRenderable>());
    renderable->batch->draw();
  }
}

void MyApp::update() {
  checkUpdateShaders();
  if (_myScene) _myScene->update(this->entities, this->events, this->getFrameDeltaTime());
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

void MyApp::setup() {
  ImGui::Initialize();
  auto window = this->getWindow();
  auto io = ImGui::GetIO();
  auto font = io.Fonts->AddFontFromFileTTF("./asset/ttf/QingNiaoHuaGuangJianMeiHei-2.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
  adjustForDPI();
  _uiSystem = std::make_shared<UISystem>();
  UISystem::init(this);
  _logger = cinder::log::makeLogger<cinder::log::LoggerSystem>();
  _logger->setLevel(cinder::log::LEVEL_INFO);
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

double MyApp::getFrameDeltaTime() {
  auto   now = this->getElapsedSeconds();
  double deltaTime = now - this->_lastTime;
  _lastTime = now;
  return deltaTime;
}

void MyApp::createProject(std::string basePath, MyProject::ProjectType projectType) {

  std::string shadersPath = basePath + "/shaders";
  std::filesystem::create_directory(shadersPath);
  std::ofstream file(shadersPath + "/default.vert");
  if (file) { file << builtIntVertShader; }
  file.close();
  file = std::ofstream(shadersPath + "/default.frag");
  if (file) { file << builtIntFragShader; }
  file.close();

  cinder::Json projJson;
  projJson["name"] = basePath.substr(basePath.find_last_of('\\'), basePath.size());
  projJson["ProjectType"] = projectType;
  projJson["shaders"] = {{"new_project", {{"VertexShader", "shaders/default.vert"}, {"FragmentShader", "shaders/default.frag"}}}};
  projJson["basePath"] = basePath;
  file = std::ofstream(basePath + "/" + basePath.substr(basePath.find_last_of('\\'), basePath.size()) + ".json");
  if (file) { file << projJson; }
  file.close();
  this->loadProject(projJson);
}

void MyApp::loadProject(const cinder::Json& projJson){
    _myProject = std::make_shared<MyProject>();
    MyProject::ProjectType projectType = projJson["ProjectType"];
    switch (projectType) {
      case MyProject::ProjectType::Proj2D: load2DProject(projJson); break;
      case MyProject::ProjectType::Proj3D: load3DProject(projJson); break;
      default: break;
    }
    _myProject->isLoaded = true;
}

void MyApp::load2DProject(const cinder::Json& projJson) {
  _myScene = this->systems.add<MySceneManageSystem>(MySceneManageSystem::SceneType::S2D, this);
  auto screenRenderable = _myScene->_rootNode->_entity.assign<ScreenRenderable>();
  //   std::shared_ptr<ScreenRenderable> screenRenderable = std::make_shared<ScreenRenderable>();
  addAssetDirectory(projJson["basePath"]);
  auto                   prog = cinder::gl::GlslProg::create(cinder::app::loadAsset(std::string(projJson["shaders"].front()["VertexShader"])),
                                                             cinder::app::loadAsset(std::string(projJson["shaders"].front()["FragmentShader"])));
  cinder::gl::VboMeshRef quadRef = cinder::gl::VboMesh::create(cinder::geom::Rect());
  screenRenderable->batch = cinder::gl::Batch::create(quadRef, prog);
  //   _myScene->_rootNode->_components.push_back(screenRenderable);
  _myProject->basePath = projJson["basePath"];
  _myProject->isLoaded = true;
  _myProject->shadersMap["new_project"] = prog;
  _popupState = PopupState::None;
  entities.create();
}

void MyApp::load3DProject(const cinder::Json& projJson) {}

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
                (_myScene->_rootNode->_entity.component<ScreenRenderable>())->batch->replaceGlslProg(shaderProgRef);
            }
        }
    }
}