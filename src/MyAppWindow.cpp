#include <MyAppWindow.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <MyApp.h>
void FileBroserWindow::showFileBrowser(const std::filesystem::path& path){
        std::vector<std::string> files;
        std::vector<std::string> directories;
        files.clear();
        directories.clear();
        std::filesystem::directory_iterator res ;
        try{
            res= std::filesystem::directory_iterator(path);
        }catch (const std::filesystem::filesystem_error& e){
            std::cout<<"Error: "<<"Access denied: "<<e.what()<<std::endl;
            return;
        }
        for(const auto& entry:std::filesystem::directory_iterator(path)){
            if(entry.is_directory()){
                directories.push_back(entry.path().filename().string());
            }else if(entry.is_regular_file()){
                files.push_back(entry.path().filename().string());
            }
        }
        for(const auto& dir: directories){
            if(ImGui::TreeNode(dir.c_str())){
                if(ImGui::IsItemClicked()){
                    _clickedPath = path.string() + "/" + dir;
                }
                showFileBrowser(path/dir);
                ImGui::TreePop();
            }
        }

        for(const auto& dir:files){
            ImGui::Button(dir.c_str());
        } 
}
bool isEmptyFile(const std::filesystem::path& path) {
    if (std::filesystem::is_directory(path)) {
        // 使用目录迭代器检查文件夹内容
        return std::filesystem::is_empty(path);
    }
    return false; // 如果不是目录，返回 false
}

void FileBroserWindow::Draw(){
    ImGui::Begin("File Browser",nullptr,ImGuiWindowFlags_NoResize);
    ImGui:ImGui::Text("Selected Path: ");
    ImGui::SameLine();
    ImGui::Text("%s", _clickedPath.c_str());
    for(const auto& rootPath:_rootPaths){
        if(ImGui::CollapsingHeader(rootPath.c_str())){
            showFileBrowser(rootPath);
        }
    }
    if(!_clickedPath.empty()){
        if(ImGui::Button("Confirm")){
            _app->createProject(_clickedPath);
        }
        ImGui::SameLine();
            if(ImGui::Button("Cancel")){
            _app->_newProject = false;
        }
    }
    ImGui::End();
}