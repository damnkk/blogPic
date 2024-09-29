#ifndef MYCOMPONENT_H
#define MYCOMPONENT_H
#include <cinder/gl/gl.h>


struct MyComponent{

};

struct ScreenRenderable: public MyComponent{
    cinder::gl::BatchRef batch; 
    std::shared_ptr<cinder::CameraPersp> camera;
};

#endif // MYCOMPONENT_H