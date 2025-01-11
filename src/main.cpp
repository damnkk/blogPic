#include "cinder/app/RendererGl.h"
#include <MyApp.h>
#include <iostream>

CINDER_APP(MyApp,
           cinder::app::RendererGl(
               cinder::app::RendererGl::Options().msaa(16) // 开启 4x MSAA
               ))