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
#include <MyProject.h>
#include <Resource/MyMaterial.h>
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

int convertCinderKeyToImguiKey(int cinderKey) {
  switch (cinderKey) {
    case ci::app::KeyEvent::KEY_BACKSPACE: return ImGuiKey_Backspace;
    case ci::app::KeyEvent::KEY_DELETE: return ImGuiKey_Delete;
    case ci::app::KeyEvent::KEY_RSHIFT: return ImGuiKey_RightShift;
    case ci::app::KeyEvent::KEY_LSHIFT: return ImGuiKey_LeftShift;
    case ci::app::KeyEvent::KEY_LCTRL: return ImGuiKey_LeftCtrl;
    case ci::app::KeyEvent::KEY_RCTRL: return ImGuiKey_RightCtrl;
    case ci::app::KeyEvent::KEY_SPACE: return ImGuiKey_Space;
    case ci::app::KeyEvent::KEY_TAB: return ImGuiKey_Tab;
    case ci::app::KeyEvent::KEY_RETURN: return ImGuiKey_Enter;
    case ci::app::KeyEvent::KEY_ESCAPE: return ImGuiKey_Escape;
    case ci::app::KeyEvent::KEY_a: return ImGuiKey_A;
    case ci::app::KeyEvent::KEY_b: return ImGuiKey_B;
    case ci::app::KeyEvent::KEY_c: return ImGuiKey_C;
    case ci::app::KeyEvent::KEY_d: return ImGuiKey_D;
    case ci::app::KeyEvent::KEY_e: return ImGuiKey_E;
    case ci::app::KeyEvent::KEY_f: return ImGuiKey_F;
    case ci::app::KeyEvent::KEY_g: return ImGuiKey_G;
    case ci::app::KeyEvent::KEY_h: return ImGuiKey_H;
    case ci::app::KeyEvent::KEY_i: return ImGuiKey_I;
    case ci::app::KeyEvent::KEY_j: return ImGuiKey_J;
    case ci::app::KeyEvent::KEY_k: return ImGuiKey_K;
    case ci::app::KeyEvent::KEY_l: return ImGuiKey_L;
    case ci::app::KeyEvent::KEY_m: return ImGuiKey_M;
    case ci::app::KeyEvent::KEY_n: return ImGuiKey_N;
    case ci::app::KeyEvent::KEY_o: return ImGuiKey_O;
    case ci::app::KeyEvent::KEY_p: return ImGuiKey_P;
    case ci::app::KeyEvent::KEY_q: return ImGuiKey_Q;
    case ci::app::KeyEvent::KEY_r: return ImGuiKey_R;
    case ci::app::KeyEvent::KEY_s: return ImGuiKey_S;
    case ci::app::KeyEvent::KEY_t: return ImGuiKey_T;
    case ci::app::KeyEvent::KEY_u: return ImGuiKey_U;
    case ci::app::KeyEvent::KEY_v: return ImGuiKey_V;
    case ci::app::KeyEvent::KEY_w: return ImGuiKey_W;
    case ci::app::KeyEvent::KEY_x: return ImGuiKey_X;
    case ci::app::KeyEvent::KEY_y: return ImGuiKey_Y;
    case ci::app::KeyEvent::KEY_z: return ImGuiKey_Z;
    case ci::app::KeyEvent::KEY_0: return ImGuiKey_0;
    case ci::app::KeyEvent::KEY_1: return ImGuiKey_1;
    case ci::app::KeyEvent::KEY_2: return ImGuiKey_2;
    case ci::app::KeyEvent::KEY_3: return ImGuiKey_3;
    case ci::app::KeyEvent::KEY_4: return ImGuiKey_4;
    case ci::app::KeyEvent::KEY_5: return ImGuiKey_5;
    case ci::app::KeyEvent::KEY_6: return ImGuiKey_6;
    case ci::app::KeyEvent::KEY_7: return ImGuiKey_7;
    case ci::app::KeyEvent::KEY_8: return ImGuiKey_8;
    case ci::app::KeyEvent::KEY_9: return ImGuiKey_9;
    case ci::app::KeyEvent::KEY_F1: return ImGuiKey_F1;
    case ci::app::KeyEvent::KEY_F2: return ImGuiKey_F2;
    case ci::app::KeyEvent::KEY_F3: return ImGuiKey_F3;
    case ci::app::KeyEvent::KEY_F4: return ImGuiKey_F4;
    case ci::app::KeyEvent::KEY_F5: return ImGuiKey_F5;
    case ci::app::KeyEvent::KEY_F6: return ImGuiKey_F6;
    case ci::app::KeyEvent::KEY_F7: return ImGuiKey_F7;
    case ci::app::KeyEvent::KEY_F8: return ImGuiKey_F8;
    case ci::app::KeyEvent::KEY_F9: return ImGuiKey_F9;
    case ci::app::KeyEvent::KEY_F10: return ImGuiKey_F10;
    case ci::app::KeyEvent::KEY_F11: return ImGuiKey_F11;
    case ci::app::KeyEvent::KEY_F12: return ImGuiKey_F12;
    case ci::app::KeyEvent::KEY_EQUALS: return ImGuiKey_Equal;
    case ci::app::KeyEvent::KEY_MINUS: return ImGuiKey_Minus;
    case ci::app::KeyEvent::KEY_COMMA: return ImGuiKey_Comma;
    case ci::app::KeyEvent::KEY_PERIOD: return ImGuiKey_Period;
    case ci::app::KeyEvent::KEY_LEFTBRACKET: return ImGuiKey_LeftBracket;
    case ci::app::KeyEvent::KEY_RIGHTBRACKET: return ImGuiKey_RightBracket;
    case ci::app::KeyEvent::KEY_SLASH: return ImGuiKey_Slash;

    case ci::app::KeyEvent::KEY_CAPSLOCK: return ImGuiKey_CapsLock;
    default: return ImGuiKey_COUNT;
  }
}

static void ImGui_ImplCinder_KeyDown(ci::app::KeyEvent& event) {
  ImGuiIO& io = ImGui::GetIO();

#if defined CINDER_LINUX
  auto character = event.getChar();
#else
  uint32_t character = event.getCharUtf32();
#endif

  io.AddKeyEvent((ImGuiKey) convertCinderKeyToImguiKey(event.getCode()), true);
  if ((ImGuiKey) convertCinderKeyToImguiKey(event.getCode()) == ImGuiKey_CapsLock) {
    // ImGui::IsNamedKeyOrMod(ImGuiKey key)
  }
  if (!event.isAccelDown() && character > 0 && character <= 255) {
    io.AddInputCharacter((char) character);
  } else if (event.getCode() != ci::app::KeyEvent::KEY_LMETA && event.getCode() != ci::app::KeyEvent::KEY_RMETA && event.isAccelDown()
             && find(sAccelKeys.begin(), sAccelKeys.end(), event.getCode()) == sAccelKeys.end()) {
    sAccelKeys.push_back(event.getCode());
  }

  io.KeyCtrl = event.isControlDown();
  io.KeyShift = event.isShiftDown();
  io.KeyAlt = event.isAltDown();
  io.KeySuper = event.isMetaDown();

  event.setHandled(io.WantCaptureKeyboard);
}

static void ImGui_ImplCinder_KeyUp(ci::app::KeyEvent& event) {
  ImGuiIO& io = ImGui::GetIO();

  io.AddKeyEvent((ImGuiKey) convertCinderKeyToImguiKey(event.getCode()), false);

  // for (auto key : sAccelKeys) { io.KeysDown[key] = false; }
  sAccelKeys.clear();

  io.KeyCtrl = event.isControlDown();
  io.KeyShift = event.isShiftDown();
  io.KeyAlt = event.isAltDown();
  io.KeySuper = event.isMetaDown();

  event.setHandled(io.WantCaptureKeyboard);
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
  sWindowConnections[window] += window->getSignalKeyDown().connect(signalPriority, ImGui_ImplCinder_KeyDown);
  sWindowConnections[window] += window->getSignalKeyUp().connect(signalPriority, ImGui_ImplCinder_KeyUp);
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
  enum { CreateProject, LoadProject } eventCase;
  bool                   openFileBrowser = false;
  MyProject::ProjectType type = MyProject::ProjectType::_Count;
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::BeginMenu("New Project")) {

        if (ImGui::MenuItem("2D Project")) {
          openFileBrowser = true;
          eventCase = CreateProject;
          type = MyProject::ProjectType::Proj2D;
        }
        if (ImGui::MenuItem("3D Project")) {
          openFileBrowser = true;
          eventCase = CreateProject;
          type = MyProject::ProjectType::Proj3D;
        }
        ImGui::EndMenu();
      }
      if (ImGui::MenuItem("Load")) {
        openFileBrowser = true;
        eventCase = LoadProject;
      }
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
  if (openFileBrowser) {
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
    bi.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_BROWSEINCLUDEFILES;
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl) {
      SHGetPathFromIDListA(pidl, folderPath);
      switch (eventCase) {
        case CreateProject: {
          _app->createProject(folderPath, type);
          break;
        }
        case LoadProject: {
          _app->loadProject(folderPath);
          break;
        }
      }
    } else {
      std::cout << "ERROR: Failed to open Folder" << std::endl;
    }
    ImGui::NewFrame();
#endif
    openFileBrowser = false;
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
static ImGuiTreeNodeFlags node_flags =
    ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
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
    if (ImGui::IsItemClicked(0) && !ImGui::IsItemToggledOpen()) {
      auto uiSystem = node->_app->_uiSystem;
      uiSystem->_selectedNode.clear();
      uiSystem->_selectedNode.push_back(node);
    }
    for (auto& child : node->getChilds()) { DrawRecursiveNode(child); }
    ImGui::TreePop();
  }
  if (node->_stateFlag & SceneNode::NodeStateFlag_Deleted) {
    node->_app->_myScene->_nodeDelectionQueue.push(node);
    return;
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
  ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
  if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags)) {
    if (ImGui::BeginTabItem("ALL")) {
      ImGui::Text("ALL");

      if (ImGui::IsMouseClicked(1)) {
        std::cout << "right click" << std::endl;
        ImGui::OpenPopup("ALL Popup");
      }
      if (ImGui::BeginPopup("ALL Popup", ImGuiPopupFlags_None)) {
        if (ImGui::MenuItem("Material")) {
          auto materialSptr = std::shared_ptr<MyMaterial>((MyMaterial*) AssetManager::instance()->createAssetByName("MyMaterial").get());
        }
        ImGui::EndPopup();
      }

      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Material")) {
      ImGui::Text("Material");
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Shader")) {
      ImGui::Text("Shader");
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Mesh")) {
      ImGui::Text("Mesh");
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }
  ImGui::End();
}