#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include <cinder/app/AppBase.h>
#include <tbb/tbb.h>
#include <cinder/app/KeyEvent.h>
#include <cinder/Log.h>
#include <cinder/Color.h>
struct MyApp:public cinder::app::App{

public:
void keyDown(cinder::app::KeyEvent event) override;
void draw() override;
};