#include <MyProject.h>
#include <MyApp.h>
#define str(x) std::to_string(x)
void MyProject::save() {
  cinder::Json Scene;
  cinder::Json UUidMap;
  auto         list = app->_myScene->transfer();
  SceneNode*   temp;
  for (const auto& obj : list) {
    Scene["NodesMap"][obj->_eid._id]["name"] = obj->_name;
    if (obj->_parent) {
      Scene["NodesMap"][obj->_eid._id]["parent"] = std::to_string(obj->_parent->_eid._id);
    } else {
      Scene["NodesMap"][obj->_eid._id]["parent"] = std::to_string(-1);
    }
    Scene["NodesMap"][obj->_eid._id]["childs"] = cinder::Json::array();
    for (const auto& child : obj->_childs) { Scene["NodesMap"][obj->_eid._id]["childs"].push_back(child->_eid._id); }
    for (const auto& component : obj->_components) {
      Scene["ComponentsMap"][uuids::to_string(component.second->_uuid)]["ComponentTypeID"] = component.first;
      Scene["ComponentsMap"][uuids::to_string(component.second->_uuid)]["Object"] = component.second->transfer();
      Scene["NodesMap"][obj->_eid._id]["components"].push_back(uuids::to_string(component.second->_uuid));
    }
  }
  std::ofstream file = std::ofstream(this->basePath + "/" + basePath.substr(basePath.find_last_of('\\'), basePath.size()) + ".json");
  if (file) {
    file << Scene;
    file.close();
  }
}