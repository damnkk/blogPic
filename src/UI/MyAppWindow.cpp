#include <UI/MyAppWindow.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <MyApp.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_opengl3.h>
#include <cinder/Clipboard.h>
#ifdef _WIN32
#include <Windows.h>
#include <shlobj.h>
#endif// _WIN32
#include <MyScene.h>
#include <ui/MyComponentEditorBase.h>

#include "component/MyFilterRenderer.h"

MyApp* MyAppWindowBase::_app = nullptr;
static bool                                                                sInitialized = false;
static bool                                                                sTriggerNewFrame = false;
static std::vector<int>                                                    sAccelKeys;
static ci::signals::ConnectionList                                         sAppConnections;
static std::unordered_map<ci::app::WindowRef, ci::signals::ConnectionList> sWindowConnections;

std::vector<std::string> ComponentLists = {"FilterRenderer", "MeshRenderer"};
static void              ImGui_ImplCinder_Shutdown() { sWindowConnections.clear(); }

static void ImGui_ImplCinder_MouseDown(ci::app::MouseEvent& event) {
  ImGuiIO& io = ImGui::GetIO();
  io.MousePos = ImVec2(event.getWindow()->toPixels(event.getPos()).x, event.getWindow()->toPixels(event.getPos()).y);
  io.MouseDown[0] = event.isLeftDown();
  io.MouseDown[1] = event.isRightDown();
  io.MouseDown[2] = event.isMiddleDown();
  event.setHandled(io.WantCaptureMouse);
}
static void ImGui_ImplCinder_MouseUp(ci::app::MouseEvent& event) {
  ImGuiIO& io = ImGui::GetIO();
  io.MouseDown[0] = false;
  io.MouseDown[1] = false;
  io.MouseDown[2] = false;
}
static void ImGui_ImplCinder_MouseWheel(ci::app::MouseEvent& event) {
  ImGuiIO& io = ImGui::GetIO();
  io.MouseWheel += event.getWheelIncrement();
  event.setHandled(io.WantCaptureMouse);
}
static void ImGui_ImplCinder_MouseMove(ci::app::MouseEvent& event) {
  ImGuiIO& io = ImGui::GetIO();
  io.MousePos = ImVec2(event.getWindow()->toPixels(event.getPos()).x, event.getWindow()->toPixels(event.getPos()).y);
  event.setHandled(io.WantCaptureMouse);
}
//! sets the right mouseDrag IO values in imgui
static void ImGui_ImplCinder_MouseDrag(ci::app::MouseEvent& event) {
  ImGuiIO& io = ImGui::GetIO();
  io.MousePos = ImVec2(event.getWindow()->toPixels(event.getPos()).x, event.getWindow()->toPixels(event.getPos()).y);
  event.setHandled(io.WantCaptureMouse);
}

static void ImGui_ImplCinder_NewFrameGuard(const ci::app::WindowRef& window);

static void ImGui_ImplCinder_Resize(const ci::app::WindowRef& window) {
  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize = ImVec2(ci::vec2(window->toPixels(window->getSize())).x, ci::vec2(window->toPixels(window->getSize())).y);

  ImGui_ImplCinder_NewFrameGuard(window);
}

static void ImGui_ImplCinder_NewFrameGuard(const ci::app::WindowRef& window) {
  if (!sTriggerNewFrame) return;

  ImGui_ImplOpenGL3_NewFrame();

  ImGuiIO& io = ImGui::GetIO();
  IM_ASSERT(io.Fonts->IsBuilt());// Font atlas needs to be built, call renderer _NewFrame() function e.g. ImGui_ImplOpenGL3_NewFrame()

  // Setup display size
  io.DisplaySize = ImVec2(window->toPixels(window->getSize()).x, window->toPixels(window->getSize()).y);

  // Setup time step
  static double g_Time = 0.0f;
  double        current_time = ci::app::getElapsedSeconds();
  io.DeltaTime = g_Time > 0.0 ? (float) (current_time - g_Time) : (float) (1.0f / 60.0f);
  g_Time = current_time;

  ImGui::NewFrame();
  ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

  sTriggerNewFrame = false;
}

static void ImGui_ImplCinder_PostDraw() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  sTriggerNewFrame = true;
}

static bool ImGui_ImplCinder_Init(const ci::app::WindowRef& window) {
  // Setup back-end capabilities flags
  ImGuiIO& io = ImGui::GetIO();
  io.BackendPlatformName = "imgui_impl_cinder";

#ifndef CINDER_LINUX
  // clipboard callbacks
  io.SetClipboardTextFn = [](void* user_data, const char* text) { ci::Clipboard::setString(std::string(text)); };
  io.GetClipboardTextFn = [](void* user_data) {
    std::string              str = ci::Clipboard::getString();
    static std::vector<char> strCopy;
    strCopy = std::vector<char>(str.begin(), str.end());
    strCopy.push_back('\0');
    return (const char*) &strCopy[0];
  };
#endif
  int signalPriority = 0;
  sWindowConnections[window] += window->getSignalMouseDown().connect(signalPriority, ImGui_ImplCinder_MouseDown);
  sWindowConnections[window] += window->getSignalMouseUp().connect(signalPriority, ImGui_ImplCinder_MouseUp);
  sWindowConnections[window] += window->getSignalMouseMove().connect(signalPriority, ImGui_ImplCinder_MouseMove);
  sWindowConnections[window] += window->getSignalMouseDrag().connect(signalPriority, ImGui_ImplCinder_MouseDrag);
  sWindowConnections[window] += window->getSignalMouseWheel().connect(signalPriority, ImGui_ImplCinder_MouseWheel);
  // sWindowConnections[window] += window->getSignalKeyDown().connect(signalPriority, ImGui_ImplCinder_KeyDown);
  // sWindowConnections[window] += window->getSignalKeyUp().connect(signalPriority, ImGui_ImplCinder_KeyUp);
  sWindowConnections[window] += window->getSignalResize().connect(signalPriority, std::bind(ImGui_ImplCinder_Resize, window));
  sWindowConnections[window] += ci::app::App::get()->getSignalUpdate().connect(std::bind(ImGui_ImplCinder_NewFrameGuard, window));
  sWindowConnections[window] += window->getSignalDraw().connect(std::bind(ImGui_ImplCinder_NewFrameGuard, window));
  sWindowConnections[window] += window->getSignalPostDraw().connect(ImGui_ImplCinder_PostDraw);
  sWindowConnections[window] += window->getSignalClose().connect([=] {
    sWindowConnections.erase(window);
    sTriggerNewFrame = false;
  });

  return true;
}

void MyAppWindowBase::init(MyApp* app) {
  if (sInitialized) return;
  _app = app;
  auto context = ImGui::CreateContext();
  // context->CurrentDpiScale = 1.0;
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  ci::app::WindowRef window = app->getWindow();
  io.DisplaySize = ImVec2(window->toPixels(window->getSize()).x, window->toPixels(window->getSize()).y);
  io.DeltaTime = 1.0f / 60.0f;
  io.WantCaptureMouse = true;

  io.IniFilename = "./imgui.ini";

#if !defined(CINDER_GL_ES)
  ImGui_ImplOpenGL3_Init("#version 150");
#else
  ImGui_ImplOpenGL3_Init();
#endif
  ImGui::StyleColorsDark();

  ImGui_ImplCinder_Init(window);
  ImGui_ImplCinder_NewFrameGuard(window);
  sTriggerNewFrame = true;
  sAppConnections += ci::app::App::get()->getSignalCleanup().connect([context]() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplCinder_Shutdown();
    ImGui::DestroyContext(context);
  });
  sInitialized = true;
  return;
}

void MainBarWindow::Draw() {
  bool                   createNewProject = false;
  MyProject::ProjectType type = MyProject::ProjectType::_Count;
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::BeginMenu("New Project")) {
        createNewProject = true;
        if (ImGui::MenuItem("2D Project")) { type = MyProject::ProjectType::Proj2D; }
        if (ImGui::MenuItem("3D Project")) { type = MyProject::ProjectType::Proj3D; }
        ImGui::EndMenu();
      }
      if (ImGui::MenuItem("Load")) {}
      if (ImGui::MenuItem("Save")) { this->_app->_myProject->save(); }
      if (ImGui::MenuItem("Exit")) { this->_app->quit(); }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Settings")) {
      if (ImGui::MenuItem("Full Screen")) {
        if (!this->_app->getWindow()->isFullScreen()) this->_app->getWindow()->setFullScreen(true);
        else
          this->_app->getWindow()->setFullScreen(false);
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
  if (createNewProject && type != MyProject::ProjectType::_Count) {
#ifdef _WIN32
    char         folderPath[256];
    const WCHAR* title = L"New Project";
    BROWSEINFO   bi = {0};
    bi.hwndOwner = NULL;
    bi.pidlRoot = CSIDL_DESKTOP;//文件夹的根目录，此处为桌面
    bi.pszDisplayName = NULL;
    bi.lpszTitle = title;// 显示位于对话框左上部的提示信息
    bi.lpfn = NULL;
    bi.iImage = 0;
    bi.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl) {
      SHGetPathFromIDListA(pidl, folderPath);
      _app->createProject(folderPath, type);
      ImGui::NewFrame();
    } else {
      std::cout << "ERROR: Failed to open Folder" << std::endl;
    }
#endif
    createNewProject = false;
  }
}

UISystem::UISystem() {
  _mainBarWindow = std::make_unique<MainBarWindow>(this);
  _sceneHierarchyWindow = std::make_unique<SceneHierarchyWindow>(this);
  _nodeInspectorWindow = std::make_unique<NodeInspectorWindow>(this);
  _resourceManageWindow = std::make_unique<ResourceManageWindow>(this);
}
UISystem::~UISystem() {}

void UISystem::Draw() {
  _mainBarWindow->Draw();
  _sceneHierarchyWindow->Draw();
  _nodeInspectorWindow->Draw();
  _resourceManageWindow->Draw();
}

void UISystem::update() {
  for (auto it = _selectedNode.begin(); it != _selectedNode.end(); it++) {
    if (!it->get()->_eid.valid()) {
      this->_selectedNode.erase(it);
      break;
    }
  }
}

static char               newName[64];
static ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow;
void                      SceneHierarchyWindow::DrawRecursiveNode(std::shared_ptr<SceneNode> node) {
  if (!node) { return; }
  if (ImGui::TreeNodeEx(node->_name.c_str(), node_flags)) {
    if (ImGui::IsItemClicked(1)) { ImGui::OpenPopup("Node Operate"); }
    if (ImGui::BeginPopup("Node Operate")) {
      if (ImGui::MenuItem("Add Node")) {
        node->addChild();
      } else if (ImGui::MenuItem("Rename")) {
        memcpy(newName, node->_name.c_str(), node->_name.size());
        node->_stateFlag = SceneNode::NodeStateFlag_Rename;
      } else if (ImGui::MenuItem("Delete")) {
        node->_stateFlag = SceneNode::NodeStateFlag_Deleted;
      }
      ImGui::EndPopup();
    }
    if (node->_stateFlag & SceneNode::NodeStateFlag_Rename) {
      ImGui::InputText("Rename", newName, 64);
      if (ImGui::Button("confirm")) {
        node->_name = std::string(newName);
        node->_stateFlag ^= SceneNode::NodeStateFlag_Rename;
        memset(newName, 0, 64);
      } else if (ImGui::Button("cancel")) {
        node->_stateFlag ^= SceneNode::NodeStateFlag_Rename;
        memset(newName, 0, 64);
      }
    }
    for (auto& child : node->getChilds()) { DrawRecursiveNode(child); }
    ImGui::TreePop();
  }
  if (node->_stateFlag & SceneNode::NodeStateFlag_Deleted) {
    node->_app->_myScene->_nodeDelectionQueue.push(node);
    return;
  }
  if (ImGui::IsItemClicked()) {
    auto uiSystem = node->_app->_uiSystem;
    uiSystem->_selectedNode.clear();
    uiSystem->_selectedNode.push_back(node);
  }
}

void SceneHierarchyWindow::Draw() {
  ImGui::Begin("Scene Hierarchy");
  if (_app->_myScene == nullptr) {
    ImGui::Text("No Scene");
    ImGui::End();
    return;
  }
  if (ImGui::TreeNodeEx("RootNode", node_flags)) {
    if (ImGui::IsItemClicked(1)) { ImGui::OpenPopup("Node Operate"); }
    if (ImGui::BeginPopup("Node Operate")) {
      if (ImGui::MenuItem("Add Node")) { _app->_myScene->_rootNode->addChild(); }
      ImGui::EndPopup();
    }
    for (auto& child : _app->_myScene->_rootNode->getChilds()) { DrawRecursiveNode(child); }
    ImGui::TreePop();
  }
  ImGui::End();
}

float degreesToRadians(float degrees) { return degrees * (M_PI / 180.0f); }

void extractRotationAngles(const glm::quat& currQuat, float& pitch, float& yaw, float& roll) {
  glm::vec3 euler = glm::eulerAngles(currQuat);
  pitch = glm::degrees(euler.x);
  yaw = glm::degrees(euler.y);
  roll = glm::degrees(euler.z);
}

glm::mat4 adjustRotation(glm::quat& currentQuat, float pitch, float yaw, float roll) {
  // 创建基于各个轴的增量四元数
  glm::quat qPitch = glm::angleAxis(degreesToRadians(pitch), glm::vec3(1, 0, 0));// 绕X轴的增量旋转
  glm::quat qYaw = glm::angleAxis(degreesToRadians(yaw), glm::vec3(0, 1, 0));    // 绕Y轴的增量旋转
  glm::quat qRoll = glm::angleAxis(degreesToRadians(roll), glm::vec3(0, 0, 1));  // 绕Z轴的增量旋转
  currentQuat = qRoll * qYaw * qPitch * currentQuat;
  // 将增量四元数应用到当前的四元数
  return glm::mat4_cast(currentQuat);// 注意乘法顺序
}

void NodeInspectorWindow::Draw() {
  ImGui::Begin("NodeInspector");
  if (this->_uiSystem->_selectedNode.empty()) {
    ImGui::Text("Please select a node first");
  } else {
    static glm::vec3 rotateVector = glm::vec3(0.0);
    auto node = this->_uiSystem->_selectedNode.front();
    ImGui::Text("Now u got a node,width name ");
    ImGui::SameLine();
    ImGui::Text("%s", node->_name.c_str());
    ImGui::CollapsingHeader("Transform", true);
    glm::vec3 translate = glm::vec3(node->_transform[3]);
    glm::vec3 scale = glm::vec3(glm::length(glm::vec3(node->_transform[0])), glm::length(glm::vec3(node->_transform[1])),
                                glm::length(glm::vec3(node->_transform[2])));
    glm::mat4 rotationMatrix = glm::mat4(node->_transform);
    rotationMatrix[0] = glm::normalize(rotationMatrix[0] / scale.x);
    rotationMatrix[1] = glm::normalize(rotationMatrix[1] / scale.y);
    rotationMatrix[2] = glm::normalize(rotationMatrix[2] / scale.z);
    glm::quat quat = glm::quat_cast(rotationMatrix);
    glm::vec3 oldTranslate = translate;
    glm::vec3 oldScale = scale;
    glm::vec3 oldRotate = rotateVector;
    ImGui::DragFloat3("Translate", glm::value_ptr(translate));
    ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.0001f, 1000.0f, "%.3f");
    ImGui::DragFloat3("Rotate", &rotateVector.x);
    if ((translate != oldTranslate) || (scale != oldScale) || (rotateVector != oldRotate)) {
      glm::mat4 transform = glm::translate(translate);
      glm::mat4 rotation = adjustRotation(quat, rotateVector.x - oldRotate.x, rotateVector.y - oldRotate.y, rotateVector.z - oldRotate.z);
      transform *= rotation;
      transform *= glm::scale(scale);
      this->_uiSystem->_selectedNode.front()->_transform = transform;
    }

    for (auto& [first, second] : node->_components) {
      auto componentEditor = MyEditorFactory::instance()->getOrCreateEditor(first).get_value<MyComponentEditorBase*>();
      if (componentEditor) {
        std::cout << "test" << std::endl;
        componentEditor->OnGUI(second.get());
      }
    }

    ImVec2 windowSize = ImGui::GetWindowSize();

    if (ImGui::ButtonEx("Add Component", ImVec2(windowSize.x, 50.0f))) { ImGui::OpenPopup("Components Lists"); }
    if (ImGui::BeginPopup("Components Lists")) {
      int selectedComponent = -1;
      for (int i = 0; i < ComponentLists.size(); ++i) {
        if (ImGui::Selectable(ComponentLists[i].c_str())) { selectedComponent = i; }
      }
      switch (selectedComponent) {
        case 0: _uiSystem->_selectedNode.front()->addComponent(MyComponentPool::instance()->createComponent<MyFilterRenderer>(_uiSystem->_app));
      }

      ImGui::EndPopup();
    }
  }
  ImGui::End();
}

void ResourceManageWindow::Draw() {
  ImGui::Begin("ResourceManager");
  ImGui::End();
}