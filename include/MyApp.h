#include <cinder/Camera.h>
#include <cinder/app/App.h>
#include <cinder/app/AppBase.h>
#include <cinder/app/RendererGl.h>
#include <cinder/gl/gl.h>
#include <tbb/tbb.h>

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

private:
  cinder::CameraPersp mainCamera;
  std::vector<cinder::gl::GlslProgRef> pipelines;
  std::vector<MyMesh> drawableList;

  float mYaw = 0.0f;
  float mPitch = 0.0f;
  cinder::ivec2 mLastMousePos;
};