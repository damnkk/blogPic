#include <UI/MyAppWindow.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <MyApp.h>
#include <imgui_internal.h>

#ifdef _WIN32
#include <Windows.h>
#include <shlobj.h>
#endif// _WIN32
#include <MyScene.h>
MyApp*                                  MyAppWindowBase::_app = nullptr;
std::vector<std::shared_ptr<SceneNode>> SceneHierarchyWindow::_selectedNode;

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
  _mainBarWindow = std::make_unique<MainBarWindow>();
  _sceneHierarchyWindow = std::make_unique<SceneHierarchyWindow>();
}
UISystem::~UISystem() {}

void UISystem::Draw() {
  _mainBarWindow->Draw();
  if (_app->_myScene == nullptr) { return; }
  _sceneHierarchyWindow->Draw();
}

static ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Selected;
void                      DrawRecursiveNode(SceneNode* node) {
  if (!node) { return; }
  if (ImGui::TreeNodeEx(node->_name.c_str(), node_flags)) {
    if (ImGui::IsItemClicked(1)) { ImGui::OpenPopup("Node Operate"); }
    if (ImGui::BeginPopup("Node Operate")) {
      if (ImGui::MenuItem("Add Node")) {
        node->addChild();
      } else if (ImGui::MenuItem("rename")) {
        static char newName[64] = "";
        memcpy(newName, node->_name.c_str(), node->_name.size());
        ////上次写到这里！！！！
      }
      ImGui::EndPopup();
    }
    for (auto& child : node->getChilds()) { DrawRecursiveNode(child.get()); }
    ImGui::TreePop();
  }
}

void SceneHierarchyWindow::Draw() {

  ImGui::Begin("Scene Hierarchy");
  if (ImGui::TreeNodeEx("RootNode", node_flags)) {
    if (ImGui::IsItemClicked(1)) { ImGui::OpenPopup("Node Operate"); }
    if (ImGui::BeginPopup("Node Operate")) {
      if (ImGui::MenuItem("Add Node")) { _app->_myScene->_rootNode->addChild(); }
      ImGui::EndPopup();
    }
    for (auto& child : _app->_myScene->_rootNode->getChilds()) {
      int a = 1;
      DrawRecursiveNode(child.get());
    }

    ImGui::TreePop();
  }
  ImGui::End();
}