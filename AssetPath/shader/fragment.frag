#version 330 core
//REFRACTION TYPE 
#define REFRACTION_TYPE_THIN                           0
#define REFRACTION_TYPE_SOLID                          1

#define SHADING_MODEL_UNLIT                             0
#define SHADING_MODEL_SPECULAR_GLOSSINESS               0   
#define SHADING_MODEL_CLOTH                             0            
#define SHADING_MODEL_SUBSURFACE                        0
#define MATERIAL_HAS_REFRACTION                         0
#define MATERIAL_HAS_SUBSURFACE_COLOR                   0
#define REFRACTION_TYPE                                 REFRACTION_TYPE_SOLID
#define MATERIAL_HAS_NORMAL                             1
#define MATERIAL_HAS_BENT_NORMAL                        0
#define MATERIAL_HAS_CLEAR_COAT                         0
#define MATERIAL_HAS_CLEAR_COAT_NORMAL                  0 
#define MATERIAL_HAS_POST_LIGHTING_COLOR                0
#define MATERIAL_HAS_ABSORPTION                         0
#define MATERIAL_HAS_TRANSMISSION                       0
#define MATERIAL_HAS_IOR                                0
#define MATERIAL_HAS_MICRO_THICKNESS                    0
#define MATERIAL_HAS_SPECULAR_FACTOR                    0
#define MATERIAL_HAS_SPECULAR_COLOR_FACTOR              1
#define MATERIAL_HAS_EMISSIVE                           1
#define MATERIAL_HAS_SHEEN_COLOR                        0
#define MATERIAL_HAS_ANISOTROPY                         0



in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
out vec4 FragColor;
uniform float exposure;

struct MaterialInputs {
    vec4  baseColor;
#if SHADING_MODEL_UNLIT == 0
#if SHADING_MODEL_SPECULAR_GLOSSINESS == 0
    float roughness;
#endif
#if SHADING_MODEL_CLOTH == 0 && SHADING_MODEL_SPECULAR_GLOSSINESS == 0
    float metallic;
    float reflectance;
#endif
    float ambientOcclusion;
#endif
    vec4  emissive;

#if SHADING_MODEL_CLOTH == 0 && SHADING_MODEL_SUBSURFACE == 0 && SHADING_MODEL_UNLIT == 0
    vec3 sheenColor;
    float sheenRoughness;
#endif

    float clearCoat;
    float clearCoatRoughness;

    float anisotropy;
    vec3  anisotropyDirection;

#if SHADING_MODEL_SUBSURFACE == 1 || MATERIAL_HAS_REFRACTION == 1
    float thickness;
#endif
#if SHADING_MODEL_SUBSURFACE == 1
    float subsurfacePower;
    vec3  subsurfaceColor;
#endif

#if SHADING_MODEL_CLOTH == 1
    vec3  sheenColor;
#if MATERIAL_HAS_SUBSURFACE_COLOR == 1
    vec3  subsurfaceColor;
#endif
#endif

#if SHADING_MODEL_SPECULAR_GLOSSINESS == 1
    vec3  specularColor;
    float glossiness;
#endif

#if MATERIAL_HAS_NORMAL == 1
    vec3  normal;
#endif
#if MATERIAL_HAS_BENT_NORMAL == 1
    vec3  bentNormal;
#endif
#if MATERIAL_HAS_CLEAR_COAT == 1 && MATERIAL_HAS_CLEAR_COAT_NORMAL == 1
    vec3  clearCoatNormal;
#endif

#if MATERIAL_HAS_POST_LIGHTING_COLOR == 1
    vec4  postLightingColor;
    float postLightingMixFactor;
#endif

#if SHADING_MODEL_CLOTH == 0 && SHADING_MODEL_SUBSURFACE == 0 && SHADING_MODEL_UNLIT == 0
#if MATERIAL_HAS_REFRACTION == 1
#if MATERIAL_HAS_ABSORPTION == 1
    vec3 absorption;
#endif
#if MATERIAL_HAS_TRANSMISSION == 1
    float transmission;
#endif
#if MATERIAL_HAS_IOR == 1
    float ior;
#endif
#if MATERIAL_HAS_MICRO_THICKNESS == 1 && (REFRACTION_TYPE == REFRACTION_TYPE_THIN)
    float microThickness;
#endif
#elif SHADING_MODEL_SPECULAR_GLOSSINESS == 0
#if MATERIAL_HAS_IOR == 1
    float ior;
#endif
#endif
#endif

#if MATERIAL_HAS_SPECULAR_FACTOR == 1
    float specularFactor;
#endif

#if MATERIAL_HAS_SPECULAR_COLOR_FACTOR == 1
    vec3 specularColorFactor;
#endif
};

void initMaterial(out MaterialInputs material) {
    material.baseColor = vec4(1.0);
#if SHADING_MODEL_UNLIT == 0
#if SHADING_MODEL_SPECULAR_GLOSSINESS == 0
    material.roughness = 1.0;
#endif
#if SHADING_MODEL_CLOTH == 0 && SHADING_MODEL_SPECULAR_GLOSSINESS == 0
    material.metallic = 0.0;
    material.reflectance = 0.5;
#endif
    material.ambientOcclusion = 1.0;
#endif
    material.emissive = vec4(vec3(0.0), 1.0);

#if SHADING_MODEL_CLOTH == 0 && SHADING_MODEL_SUBSURFACE == 0 && SHADING_MODEL_UNLIT == 0
#if MATERIAL_HAS_SHEEN_COLOR == 1
    material.sheenColor = vec3(0.0);
    material.sheenRoughness = 0.0;
#endif
#endif

#if MATERIAL_HAS_CLEAR_COAT == 1
    material.clearCoat = 1.0;
    material.clearCoatRoughness = 0.0;
#endif

#if MATERIAL_HAS_ANISOTROPY == 1
    material.anisotropy = 0.0;
    material.anisotropyDirection = vec3(1.0, 0.0, 0.0);
#endif

#if SHADING_MODEL_SUBSURFACE == 1 || MATERIAL_HAS_REFRACTION == 1
    material.thickness = 0.5;
#endif
#if SHADING_MODEL_SUBSURFACE == 1
    material.subsurfacePower = 12.234;
    material.subsurfaceColor = vec3(1.0);
#endif

#if SHADING_MODEL_CLOTH == 1
    material.sheenColor = sqrt(material.baseColor.rgb);
#if MATERIAL_HAS_SUBSURFACE_COLOR == 1
    material.subsurfaceColor = vec3(0.0);
#endif
#endif

#if SHADING_MODEL_SPECULAR_GLOSSINESS == 1
    material.glossiness = 0.0;
    material.specularColor = vec3(0.0);
#endif

#if MATERIAL_HAS_NORMAL == 1
    material.normal = vec3(0.0, 0.0, 1.0);
#endif
#if MATERIAL_HAS_BENT_NORMAL == 1
    material.bentNormal = vec3(0.0, 0.0, 1.0);
#endif
#if MATERIAL_HAS_CLEAR_COAT == 1 && MATERIAL_HAS_CLEAR_COAT_NORMAL == 1
    material.clearCoatNormal = vec3(0.0, 0.0, 1.0);
#endif

#if MATERIAL_HAS_POST_LIGHTING_COLOR == 1
    material.postLightingColor = vec4(0.0);
    material.postLightingMixFactor = 1.0;
#endif

#if SHADING_MODEL_CLOTH == 0 && SHADING_MODEL_SUBSURFACE == 0 && SHADING_MODEL_UNLIT == 0
#if MATERIAL_HAS_REFRACTION == 1
#if MATERIAL_HAS_ABSORPTION == 1
    material.absorption = vec3(0.0);
#endif
#if MATERIAL_HAS_TRANSMISSION == 1
    material.transmission = 1.0;
#endif
#if MATERIAL_HAS_IOR == 1
    material.ior = 1.5;
#endif
#if MATERIAL_HAS_MICRO_THICKNESS == 1 && (REFRACTION_TYPE == REFRACTION_TYPE_THIN)
    material.microThickness = 0.0;
#endif
#elif SHADING_MODEL_SPECULAR_GLOSSINESS == 0
#if MATERIAL_HAS_IOR == 1
    material.ior = 1.5;
#endif
#endif
#endif

#if MATERIAL_HAS_SPECULAR_FACTOR == 1
    material.specularFactor = 1.0;
#endif

#if MATERIAL_HAS_SPECULAR_COLOR_FACTOR == 1
    material.specularColorFactor = vec3(1.0);
#endif

}

struct PixelParams {
    vec3  diffuseColor;
    float perceptualRoughness;
    float perceptualRoughnessUnclamped;
    vec3  f0;
#if MATERIAL_HAS_SPECULAR_COLOR_FACTOR == 1 || MATERIAL_HAS_SPECULAR_FACTOR == 1
    float f90;
    float specular;
    vec3  specularColor;
#endif
    float roughness;
    vec3  dfg;
    vec3  energyCompensation;

#if MATERIAL_HAS_CLEAR_COAT == 1
    float clearCoat;
    float clearCoatPerceptualRoughness;
    float clearCoatRoughness;
#endif

#if MATERIAL_HAS_SHEEN_COLOR == 1
    vec3  sheenColor;
#if SHADING_MODEL_CLOTH == 0
    float sheenRoughness;
    float sheenPerceptualRoughness;
    float sheenScaling;
    float sheenDFG;
#endif
#endif

#if MATERIAL_HAS_ANISOTROPY == 1
    vec3  anisotropicT;
    vec3  anisotropicB;
    float anisotropy;
#endif

#if SHADING_MODEL_SUBSURFACE == 1 || MATERIAL_HAS_REFRACTION == 1
    float thickness;
#endif
#if SHADING_MODEL_SUBSURFACE == 1
    vec3  subsurfaceColor;
    float subsurfacePower;
#endif

#if SHADING_MODEL_CLOTH == 1 && MATERIAL_HAS_SUBSURFACE_COLOR == 1
    vec3  subsurfaceColor;
#endif

#if MATERIAL_HAS_REFRACTION == 1
    float etaRI;
    float etaIR;
    float transmission;
    float uThickness;
    vec3  absorption;
#endif
};

vec4 evaluateMaterial(MaterialInputs material) {
    vec4 color = material.baseColor;
    return color;
}


void addEmissive(const MaterialInputs material, inout vec4 color) {
#if MATERIAL_HAS_EMISSIVE==1
    highp vec4 emissive = material.emissive;
    highp float attenuation = mix(1.0, exposure, emissive.w);
    attenuation *= color.a;
    color.rgb += emissive.rgb * attenuation;
#endif
}

void getSpecularPixelParams(const MaterialInputs material, inout PixelParams pixel) {
#if MATERIAL_HAS_SPECULAR_FACTOR==1
    pixel.specular = material.specularFactor;
#endif

#if MATERIAL_HAS_SPECULAR_COLOR_FACTOR==1
    pixel.specularColor = material.specularColorFactor;
#endif
}

void getPixelParams(const MaterialInputs material, out PixelParams pixel) {
    getSpecularPixelParams(material, pixel);
    // getCommonPixelParams(material, pixel);
    // getSheenPixelParams(material, pixel);
    // getClearCoatPixelParams(material, pixel);
    // getRoughnessPixelParams(material, pixel);
    // getSubsurfacePixelParams(material, pixel);
    // getAnisotropyPixelParams(material, pixel);
    // getEnergyCompensationPixelParams(pixel);
}

vec4 avaluateLights(MaterialInputs material){
    PixelParams pixel;
    getPixelParams(material,pixel);

    return vec4(1.0);
}

void main(){

    MaterialInputs inputs;
    initMaterial(inputs);
    vec3 listDir = normalize(vec3(1.0,1.0,1.0));
    
    FragColor = vec4(vec3(FragPos.xyz),1.0);
    FragColor = evaluateMaterial(inputs);
    // FragColor = vec4(1.0);
}