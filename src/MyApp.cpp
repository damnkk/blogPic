#include <MyApp.h>

void MyApp::keyDown(cinder::app::KeyEvent event){
    if(event.getCode()==cinder::app::KeyEvent::KEY_ESCAPE){
       std::cout<<"Escape key pressed. Exiting..."<<std::endl;
       this->quit();
    }
}

void MyApp::draw(){
    glClearColor(0.2,0.2,0.2,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    std::cout<<this->getAverageFps()<<'\r';

}