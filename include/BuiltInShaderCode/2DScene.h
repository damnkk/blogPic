#include <string>
std::string builtIntVertShader = R"(
#version 460  
layout (location = 0) in vec4   ciPosition;
layout (location = 1) in vec2   ciTexCoord0;
out vec2 uv;
void main(){   
    uv = vec2(ciPosition.xy)+0.5;
    gl_Position = vec4(ciPosition.x*2.0, ciPosition.y*2.0, ciPosition.z, 1.0);
})";
std::string builtIntFragShader = R"(
#version 460 
out vec4 FragColor;
in vec2 uv;
void main(){   
    FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
)";
