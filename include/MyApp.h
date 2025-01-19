#include <cinder/Camera.h>
#include <cinder/app/App.h>
#include <cinder/app/AppBase.h>
#include <cinder/app/RendererGl.h>
#include <cinder/gl/gl.h>
#include <stb_image.h>
#include <tbb/tbb.h>

class Texture {
public:
  Texture() {}

  ~Texture() {
    if (_textureID != 0) {
      glDeleteTextures(1, &_textureID);
    }
  }

  Texture &generate(GLenum target = GL_TEXTURE_2D) {
    glGenTextures(1, &_textureID);
    _target = target;
    glBindTexture(GL_TEXTURE_2D, _textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    return *this;
  }

  void bind() { glBindTexture(_target, _textureID); }
  void bind(GLuint slotIndex) {
    _slotIdx = slotIndex;
    glActiveTexture(GL_TEXTURE0 + _slotIdx);
    glBindTexture(_target, _textureID);
  }

  void unbind() const { glBindTexture(_target, 0); }
  GLuint getSlotIdx() const { return _slotIdx; }

  Texture &loadFromPath(std::string path) {
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data =
        stbi_load(path.c_str(), &width, &height, &nrChannels, 4);
    if (data) {
      glBindTexture(_target, _textureID);
      glTexImage2D(_target, 0, GL_RGBA, width, height, 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, data);
      // glGenerateMipmap(_target);
    } else {
      std::cerr << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    return *this;
  }

  GLuint getID() const { return _textureID; }

private:
  GLuint _slotIdx;
  GLuint _textureID;
  GLenum _target;
};
class MyMesh {
public:
  cinder::gl::VaoRef _vao;
  cinder::gl::VboRef _vbo;
  cinder::gl::VboRef _ebo;
  cinder::gl::GlslProgRef _shader;
  uint32_t _vertexCnt;
  uint32_t _indexCnt;
  void draw() {
    cinder::gl::drawElements(GL_TRIANGLES, _indexCnt, GL_UNSIGNED_INT, 0);
  }
};

struct MyApp : public cinder::app::App {

  void setup() override;
  void update() override;
  void draw() override;
  void keyDown(cinder::app::KeyEvent event) override;
  void keyUp(cinder::app::KeyEvent event) override;
  void resize() override;

  // void mouseMove(cinder::app::MouseEvent event) override;
  void mouseDrag(cinder::app::MouseEvent event) override;
  enum PipelineType { Pip_lit, Pip_SkyBox, _count };

  struct SkyBox {
    cinder::gl::BatchRef _batch;
    std::vector<Texture> _envTextures;
  } _skybox;

private:
  cinder::CameraPersp mainCamera;
  std::vector<cinder::gl::GlslProgRef> pipelines;
  std::vector<MyMesh> drawableList;

  float mYaw = 0.0f;
  float mPitch = 0.0f;
  cinder::ivec2 mLastMousePos;
};