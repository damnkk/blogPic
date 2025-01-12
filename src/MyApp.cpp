// clang-format off
#include <MyApp.h>
#include <cinder/ObjLoader.h>
#include <cinder/log.h>
#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
// clang-format on

std::vector<MyMesh> LoadGltfModel(std::string path) {
  tinygltf::TinyGLTF loader;
  tinygltf::Model model;
  std::string error;
  std::string warn;
  auto res = loader.LoadASCIIFromFile(&model, &error, &warn, path);
  if (!res) {
    CI_LOG_E("ERROR " << error);
    CI_LOG_W("WARN " << warn);
    return {};
  }
  std::vector<MyMesh> meshLists;
  auto meshes = model.meshes;
  for (auto &mesh : meshes) {
    for (auto &primitive : mesh.primitives) {
      MyMesh subMesh;
      subMesh._vao = cinder::gl::Vao::create();
      subMesh._vao->bind();
      auto vertexBufferAccessor =
          model.accessors[primitive.attributes["POSITION"]];
      auto normalBufferAccessor =
          model.accessors[primitive.attributes["NORMAL"]];
      auto indexBufferAccessor = model.accessors[primitive.indices];
      auto UVBufferAccessor =
          model.accessors[primitive.attributes["TEXCOORD_0"]];

      auto vertexBufferView =
          model.bufferViews[vertexBufferAccessor.bufferView];
      auto normalBufferView =
          model.bufferViews[normalBufferAccessor.bufferView];
      auto indexBufferView = model.bufferViews[indexBufferAccessor.bufferView];
      auto UVBufferView = model.bufferViews[UVBufferAccessor.bufferView];

      auto indexDataPtr = model.buffers[indexBufferView.buffer].data.data() +
                          indexBufferView.byteOffset +
                          indexBufferAccessor.byteOffset;
      auto positionDataPtr =
          model.buffers[vertexBufferView.buffer].data.data() +
          vertexBufferView.byteOffset + vertexBufferAccessor.byteOffset;
      auto normalDataPtr = model.buffers[normalBufferView.buffer].data.data() +
                           normalBufferView.byteOffset +
                           normalBufferAccessor.byteOffset;
      auto UVDataPtr = model.buffers[UVBufferView.buffer].data.data() +
                       UVBufferView.byteOffset + UVBufferAccessor.byteOffset;

      subMesh._vbo = cinder::gl::Vbo::create(
          GL_ARRAY_BUFFER,
          vertexBufferAccessor.count * sizeof(cinder::vec3) +
              normalBufferAccessor.count * sizeof(cinder::vec3) +
              UVBufferAccessor.count * sizeof(cinder::vec2),
          nullptr, GL_STATIC_DRAW);
      subMesh._vbo->bind();
      subMesh._vbo->bufferSubData(
          0, vertexBufferAccessor.count * sizeof(cinder::vec3),
          positionDataPtr);
      subMesh._vbo->bufferSubData(
          vertexBufferAccessor.count * sizeof(cinder::vec3),
          normalBufferAccessor.count * sizeof(cinder::vec3), normalDataPtr);
      subMesh._vbo->bufferSubData(
          vertexBufferAccessor.count * sizeof(cinder::vec3) +
              normalBufferAccessor.count * sizeof(cinder::vec3),
          UVBufferAccessor.count * sizeof(cinder::vec2), UVDataPtr);

      if (indexBufferView.byteLength / indexBufferAccessor.count == 2) {
        std::vector<uint16_t> indexBufferObject(indexBufferAccessor.count);
        memcpy(indexBufferObject.data(), indexDataPtr,
               indexBufferView.byteLength);
        std::vector<uint32_t> indexBufferObject32(indexBufferAccessor.count);
        std::transform(indexBufferObject.begin(), indexBufferObject.end(),
                       indexBufferObject32.begin(),
                       [](uint16_t val) { return static_cast<uint32_t>(val); });
        subMesh._ebo = cinder::gl::Vbo::create(
            GL_ELEMENT_ARRAY_BUFFER,
            indexBufferObject32.size() * sizeof(uint32_t),
            indexBufferObject32.data(), GL_STATIC_DRAW);
      } else if (indexBufferView.byteLength / indexBufferAccessor.count == 4) {
        subMesh._ebo = cinder::gl::Vbo::create(GL_ELEMENT_ARRAY_BUFFER,
                                               indexBufferAccessor.count *
                                                   sizeof(uint32_t),
                                               indexDataPtr, GL_STATIC_DRAW);
      }

      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

      GLintptr normalOffset = vertexBufferAccessor.count * sizeof(cinder::vec3);
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)normalOffset);

      GLintptr uvOffset =
          normalOffset + normalBufferAccessor.count * sizeof(cinder::vec3);

      glEnableVertexAttribArray(2);
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void *)uvOffset);
      subMesh._indexCnt = indexBufferAccessor.count;
      subMesh._vao->unbind();
      meshLists.push_back(subMesh);
    }
  }
  return meshLists;
}
void MyApp::setup() {
  this->setFrameRate(140);
  this->setWindowSize(glm::ivec2(1920, 1080));
  mainCamera = cinder::CameraPersp(1920, 1920, 90.0f);
  mainCamera.setViewDirection(cinder::vec3(0, 0, -1));
  mainCamera.setEyePoint(cinder::vec3(0.0, 0.0, 2.0));
  mainCamera.setNearClip(0.1);
  mainCamera.setFarClip(1000000.0);
  this->addAssetDirectory(u8"./AssetPath/shader/");
  tinygltf::Model model;
  drawableList = LoadGltfModel("./DamagedHelmet/DamagedHelmet.gltf");
  pipelines.resize((int)PipelineType::_count);
  try {

    pipelines[PipelineType::Pip_lit] = cinder::gl::GlslProg::create(
        cinder::gl::GlslProg::Format()
            .vertex(cinder::app::loadAsset("vertex.vert"))
            .fragment(cinder::app::loadAsset("fragment.frag")));
  } catch (...) {
    CI_LOG_E("Failed to compile updated shaders");

    pipelines[PipelineType::Pip_lit] = nullptr;
  }

  cinder::gl::enableDepthRead();
  cinder::gl::enableDepthWrite();
}

float rotate = 0.0;

void MyApp::update() {

  // CI_LOG_I(this->getAverageFps());
  static auto startTime = std::chrono::steady_clock::now();
  auto now = std::chrono::steady_clock::now();
  float elapsed = std::chrono::duration<float>(now - startTime).count();
  float angle = elapsed * 0.1f;
  cinder::mat4 rotateMat =
      glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0));
  if (pipelines[PipelineType::Pip_lit]) {

    this->pipelines[PipelineType::Pip_lit]->uniform("model", rotateMat);
    this->pipelines[PipelineType::Pip_lit]->uniform("view",
                                                    mainCamera.getViewMatrix());
    this->pipelines[PipelineType::Pip_lit]->uniform(
        "projection", mainCamera.getProjectionMatrix());
  }

  {
    static std::filesystem::file_time_type lastVertWrite;
    static std::filesystem::file_time_type lastFragWrite;
    std::string basePath = "./AssetPath/shader/";
    auto vertPath = basePath + "vertex.vert";
    auto fragPath = basePath + "fragment.frag";
    if (std::filesystem::exists(vertPath) &&
        std::filesystem::exists(fragPath)) {
      auto newVertWrite = std::filesystem::last_write_time(vertPath);
      auto newFragWrite = std::filesystem::last_write_time(fragPath);
      if (newVertWrite != lastVertWrite || newFragWrite != lastFragWrite) {
        try {
          auto newProg = cinder::gl::GlslProg::create(
              cinder::gl::GlslProg::Format()
                  .vertex(cinder::app::loadAsset("vertex.vert"))
                  .fragment(cinder::app::loadAsset("fragment.frag")));
          pipelines[PipelineType::Pip_lit] = newProg;
          lastVertWrite = newVertWrite;
          lastFragWrite = newFragWrite;
        } catch (...) {
          CI_LOG_E("Failed to compile updated shaders");
          pipelines[PipelineType::Pip_lit] = nullptr;
        }
      }
    }
  }
}

void MyApp::draw() {
  // cinder::gl::clear();
  cinder::gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  if (pipelines[PipelineType::Pip_lit]) {

    cinder::gl::clearColor(cinder::ColorA(1.0f, 1.0f, 0.0f, 1.0f));
    for (auto &mesh : drawableList) {
      mesh._vao->bind();
      mesh._ebo->bind();
      this->pipelines[PipelineType::Pip_lit]->bind();
      mesh.draw();
    }
  } else {
    cinder::gl::clearColor(cinder::ColorA(1.0f, 0.0f, 1.0f, 1.0f));
  }
}

void MyApp::keyDown(cinder::app::KeyEvent event) {
  CI_LOG_I("Key " << event.getChar() << " pressed");
  if (event.getCode() == cinder::app::KeyEvent::KEY_a) {
    auto newEyePoint = this->mainCamera.getEyePoint() + cinder::vec3(0.1, 0, 0);
    if (newEyePoint != this->mainCamera.getEyePoint()) {
      this->mainCamera.setEyePoint(newEyePoint);
    }
  }
  if (event.getCode() == cinder::app::KeyEvent::KEY_s) {
    auto newEyePoint =
        this->mainCamera.getEyePoint() + cinder::vec3(0, 0., 0.1);
    if (newEyePoint != this->mainCamera.getEyePoint()) {
      this->mainCamera.setEyePoint(newEyePoint);
    }
  }
  if (event.getCode() == cinder::app::KeyEvent::KEY_w) {
    auto newEyePoint =
        this->mainCamera.getEyePoint() + cinder::vec3(0, 0.0, -0.1);
    if (newEyePoint != this->mainCamera.getEyePoint()) {
      this->mainCamera.setEyePoint(newEyePoint);
    }
  }
  if (event.getCode() == cinder::app::KeyEvent::KEY_d) {
    auto newEyePoint =
        this->mainCamera.getEyePoint() + cinder::vec3(-0.1, 0, 0);
    if (newEyePoint != this->mainCamera.getEyePoint()) {
      this->mainCamera.setEyePoint(newEyePoint);
    }
  }
  if (event.getCode() == cinder::app::KeyEvent::KEY_SPACE) {
    auto newEyePoint = this->mainCamera.getEyePoint() + cinder::vec3(0, 0.1, 0);
    if (newEyePoint != this->mainCamera.getEyePoint()) {
      this->mainCamera.setEyePoint(newEyePoint);
    }
  }
  if (event.getCode() == cinder::app::KeyEvent::KEY_LCTRL) {
    auto newEyePoint =
        this->mainCamera.getEyePoint() + cinder::vec3(0, -0.1, 0);
    if (newEyePoint != this->mainCamera.getEyePoint()) {
      this->mainCamera.setEyePoint(newEyePoint);
    }
  }
}

void MyApp::keyUp(cinder::app::KeyEvent event) {
  if (event.getCode() == cinder::app::KeyEvent::KEY_a) {
    CI_LOG_I("Key 'A' released");
    // 在这里处理按键 'A' 被释放的逻辑
  }
  // 可以添加更多按键检测逻辑
}

void MyApp::resize() { mainCamera.setAspectRatio(getWindowAspectRatio()); }

void MyApp::mouseDrag(cinder::app::MouseEvent event) {}