#include <iostream>
#include <MyApp.h>

CINDER_APP(MyApp, cinder::app::RendererGl,[](cinder::app::App::Settings* settings){
    settings->setWindowSize(glm::ivec2(1920,1080));
    settings->setWindowPos(glm::ivec2(200,200));
})