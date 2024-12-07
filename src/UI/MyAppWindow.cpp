#include <UI/MyAppWindow.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <MyApp.h>
#include <imgui_internal.h>

#ifdef _WIN32
#include <Windows.h>
#include <shlobj.h>
#endif// _WIN32
#include <MyScene.h>
MyApp* MyAppWindowBase::_app = nullptr;

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
      if (ImGui::MenuItem("Save")) {}
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
}
UISystem::~UISystem() {}

void UISystem::Draw() {
  _mainBarWindow->Draw();
  _sceneHierarchyWindow->Draw();
  _nodeInspectorWindow->Draw();
}

static char               newName[64];
static ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Selected;
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
  if (ImGui::IsItemClicked(0)) {
    auto uiSystem = node->_app->_uiSystem;
    uiSystem->_selectedNode.clear();
    uiSystem->_selectedNode.push_back(node);
  }
}

void SceneHierarchyWindow::Draw() {
  if (_app->_myScene == nullptr) {
    ImGui::Text("No Scene");
    return;
  }
  ImGui::Begin("Scene Hierarchy");
  if (ImGui::TreeNodeEx("RootNode", node_flags)) {
    if (ImGui::IsItemClicked(1)) { ImGui::OpenPopup("Node Operate"); }
    if (ImGui::BeginPopup("Node Operate")) {
      if (ImGui::MenuItem("Add Node")) { _app->_myScene->_rootNode->addChild(); }
      ImGui::EndPopup();
    }
    for (auto& child : _app->_myScene->_rootNode->getChilds()) {
      int a = 1;
      DrawRecursiveNode(child);
    }
    ImGui::TreePop();
  }
  ImGui::End();
}

void NodeInspectorWindow::Draw() {
  ImGui::Begin("NodeInspector");
  if (this->_uiSystem->_selectedNode.empty()) {
    ImGui::Text("Please select a node first");
  } else {
    auto node = this->_uiSystem->_selectedNode.front();
    ImGui::Text("Now u got a node,width name ");
    ImGui::SameLine();
    ImGui::Text("%s", node->_name.c_str());
    if (ImGui::CollapsingHeader("Transform")) {
      glm::vec3 translate = glm::vec3(node->_transform[3]);
      glm::vec3 scale = glm::vec3(glm::length(glm::vec3(node->_transform[0])), glm::length(glm::vec3(node->_transform[1])),
                                  glm::length(glm::vec3(node->_transform[2])));
      glm::mat4 rotationMatrix = glm::mat4(node->_transform);
      rotationMatrix[0] = glm::normalize(rotationMatrix[0] / scale.x);
      rotationMatrix[1] = glm::normalize(rotationMatrix[1] / scale.y);
      rotationMatrix[2] = glm::normalize(rotationMatrix[2] / scale.z);
      glm::quat q = glm::quat_cast(rotationMatrix);
    }
  }
  ImGui::End();
}