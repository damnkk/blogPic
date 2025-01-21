#version 330 core
//REFRACTION TYPE 
#define REFRACTION_TYPE_THIN                           0
#define REFRACTION_TYPE_SOLID                          1

#define REFRACTION_TYPE                                 REFRACTION_TYPE_SOLID
#define SHADING_MODEL_UNLIT 1
#define SHADING_MODEL_SPECULAR_GLOSSINESS 0
#define SHADING_MODEL_CLOTH 0
#define SHADING_MODEL_SUBSURFACE                        0
#define MATERIAL_HAS_REFRACTION                         0
#define MATERIAL_HAS_REFLECTANCE                        0
// screen-space reflections
#define MATERIAL_HAS_REFLECTIONS 1
#define MATERIAL_HAS_SUBSURFACE_COLOR                   0
#define MATERIAL_HAS_NORMAL                             1
#define MATERIAL_HAS_BENT_NORMAL 1
#define MATERIAL_HAS_CLEAR_COAT                         0
#define MATERIAL_HAS_CLEAR_COAT_NORMAL 0
#define MATERIAL_HAS_POST_LIGHTING_COLOR                0
#define MATERIAL_HAS_ABSORPTION                         0
#define MATERIAL_HAS_TRANSMISSION                       0
#define MATERIAL_HAS_IOR                                0
#define MATERIAL_HAS_MICRO_THICKNESS                    0
#define MATERIAL_HAS_SPECULAR_FACTOR                    0
#define MATERIAL_HAS_SPECULAR_COLOR_FACTOR 0
#define MATERIAL_HAS_EMISSIVE                           1
#define MATERIAL_HAS_SHEEN_COLOR                        0
#define MATERIAL_HAS_ANISOTROPY                         0
#define BLEND_MODE_OPAQUE 1

#define MIN_PERCEPTUAL_ROUGHNESS 0.089
#define MIN_ROUGHNESS            0.007921
#define saturate(x)        clamp(x, 0.0, 1.0)

#define MIN_N_DOT_V 1e-4


in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
out vec4 FragColor;
vec3 shading_reflected;  // reflection of view about normal
vec3 shading_normal;
vec3 shading_view;
float shading_NoV;
uniform float exposure;
uniform vec3 cameraPos;
uniform mat4 view;
uniform mat4 projection;
uniform sampler2D sampler0_iblDFG;
uniform sampler2D sampler0_iblSpecular;
uniform sampler2D sampler0_ssr;
// frame uniform
float ssrDistance = 20.0;
float refractionLodOffset = 6.87707;
// end frame uniform

vec2 shading_normalizedViewportCoord;

struct SSAOInterpolationCache {
  highp vec4 weights;
#if BLEND_MODE_OPAQUE == 1 || BLEND_MODE_MASKED == 1 || \
    MATERIAL_HAS_REFLECTIONS == 1
  highp vec2 uv;
#endif
};

// math functions
float sq(float x) {
    return x * x;
}

float max3(const vec3 v) {
    return max(v.x, max(v.y, v.z));
}

float vmax(const vec2 v) {
    return max(v.x, v.y);
}

float vmax(const vec3 v) {
    return max(v.x, max(v.y, v.z));
}

float vmax(const vec4 v) {
    return max(max(v.x, v.y), max(v.y, v.z));
}

vec4 mulMat4x4Float3(const highp mat4 m, const highp vec3 v) {
  return v.x * m[0] + (v.y * m[1] + (v.z * m[2] + m[3]));
}

/**
 * Returns the minimum component of the specified vector.
 *
 * @public-api
 */
float min3(const vec3 v) {
    return min(v.x, min(v.y, v.z));
}

float vmin(const vec2 v) {
    return min(v.x, v.y);
}

float vmin(const vec3 v) {
    return min(v.x, min(v.y, v.z));
}

float vmin(const vec4 v) {
    return min(min(v.x, v.y), min(v.y, v.z));
}

float clampNoV(float NoV) {
    // Neubelt and Pettineo 2013, "Crafting a Next-gen Material Pipeline for The Order: 1886"
    return max(NoV, MIN_N_DOT_V);
}

vec3 computeDiffuseColor(const vec4 baseColor, float metallic) {
    return baseColor.rgb * (1.0 - metallic);
}

vec3 computeF0(const vec4 baseColor, float metallic, float reflectance) {
    return baseColor.rgb * metallic + (reflectance * (1.0 - metallic));
}

float computeDielectricF0(float reflectance) {
    return 0.16 * reflectance * reflectance;
}

float computeMetallicFromSpecularColor(const vec3 specularColor) {
    return max3(specularColor);
}

float computeRoughnessFromGlossiness(float glossiness) {
    return 1.0 - glossiness;
}

float perceptualRoughnessToRoughness(float perceptualRoughness) {
    return perceptualRoughness * perceptualRoughness;
}

float roughnessToPerceptualRoughness(float roughness) {
    return sqrt(roughness);
}

float iorToF0(float transmittedIor, float incidentIor) {
    return sq((transmittedIor - incidentIor) / (transmittedIor + incidentIor));
}

float f0ToIor(float f0) {
    float r = sqrt(f0);
    return (1.0 + r) / (1.0 - r);
}

vec3 f0ClearCoatToSurface(const vec3 f0) {

    return saturate(f0 * (f0 * (0.941892 - 0.263008 * f0) + 0.346479) - 0.0285998);
}

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

//------------------------------------------------------------------------------
// IBL specular
//------------------------------------------------------------------------------

vec3 specularDFG(const PixelParams pixel){
#if SHADING_MODEL_CLOTH==1
    return pixel.f0 * pixel.dfg.z;
#else
#if MATERIAL_HAS_SPECULAR_COLOR_FACTOR==1 ||MATERIAL_HAS_SPECULAR_FACTOR==1
    return mix(pixel.dfg.xxx, pixel.dfg.yyy, pixel.f0) * pixel.specular;
#else
    return mix(pixel.dfg.xxx, pixel.dfg.yyy, pixel.f0);
#endif
#endif
}


float perceptualRoughnessToLod(float perceptualRoughness) {
    // The mapping below is a quadratic fit for log2(perceptualRoughness)+iblRoughnessOneLevel when
    // iblRoughnessOneLevel is 4. We found empirically that this mapping works very well for
    // a 256 cubemap with 5 levels used. But also scales well for other iblRoughnessOneLevel values.
    // return frameUniforms.iblRoughnessOneLevel * perceptualRoughness * (2.0 - perceptualRoughness);
    return 1.2;
}

vec3 prefilteredRadiance(const vec3 r, float perceptualRoughness) {
    // float lod = perceptualRoughnessToLod(perceptualRoughness);
    // return textureLod(sampler0_iblSpecular, r, lod).xyz;
    return vec3(0.5);
}

vec3 prefilteredRadiance(const vec3 r, float roughness, float offset) {
    // float lod = frameUniforms.iblRoughnessOneLevel * roughness;
    // return textureLod(sampler0_iblSpecular, r, lod + offset).xyz;
    return vec3(0.5);
}


vec3 getSpecularDominantDirection(const vec3 n, const vec3 r, float roughness) {
    return mix(r, n, roughness * roughness);
}

vec3 getReflectedVector(const PixelParams pixel, const vec3 v, const vec3 n) {
#if MATERIAL_HAS_ANISOTROPY==1
    vec3  anisotropyDirection = pixel.anisotropy >= 0.0 ? pixel.anisotropicB : pixel.anisotropicT;
    vec3  anisotropicTangent  = cross(anisotropyDirection, v);
    vec3  anisotropicNormal   = cross(anisotropicTangent, anisotropyDirection);
    float bendFactor          = abs(pixel.anisotropy) * saturate(5.0 * pixel.perceptualRoughness);
    vec3  bentNormal          = normalize(mix(n, anisotropicNormal, bendFactor));

    vec3 r = reflect(-v, bentNormal);
#else
    vec3 r = reflect(-v, n);
#endif
    return r;
}

vec3 getReflectedVector(const PixelParams pixel, const vec3 n) {
#if MATERIAL_HAS_ANISOTROPY==1
    vec3 r = getReflectedVector(pixel, shading_view, n);
#else
    vec3 r = shading_reflected;
#endif
    return getSpecularDominantDirection(n, r, pixel.roughness);
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

void getCommonPixelParams(const MaterialInputs material, inout PixelParams pixel) {
    vec4 baseColor = material.baseColor;
#if SHADING_MODEL_UNLIT == 0
#if SHADING_MODEL_SPECULAR_GLOSSINESS==1
    vec3 specularColor = material.specularColor;
    float metallic = computeMetallicFromSpecularColor(specularColor);
    pixel.diffuseColor = computeDiffuseColor(baseColor, metallic);
    pixel.f0 = specularColor;
#elif SHADING_MODEL_CLOTH==0
    pixel.diffuseColor = computeDiffuseColor(baseColor, material.metallic);
#if SHADING_MODEL_SUBSURFACE==0&&(MATERIAL_HAS_REFLECTANCE==0&&MATERIAL_HAS_IOR==1)
    float reflectance = iorToF0(max(1.0, material.ior), 1.0);
#else
    float reflectance = computeDielectricF0(material.reflectance);
#endif  // SHADING_MODEL_SUBSURFACE==0&&(MATERIAL_HAS_REFLECTANCE==0&&MATERIAL_HAS_IOR==1)
#if MATERIAL_HAS_SPECULAR_FACTOR==0 && MATERIAL_HAS_SPECULAR_COLOR_FACTOR==0
    pixel.f0 = computeF0(baseColor, material.metallic, reflectance);
#else
    vec3 dielectricSpecularF0 = vec3(0.0);
    float dielectricSpecularF90 = 0.0;
#if MATERIAL_HAS_SPECULAR_COLOR_FACTOR == 1
    dielectricSpecularF0 = min(reflectance * material.specularColorFactor, vec3(1.0));
#endif  // MATERIAL_HAS_SPECULAR_COLOR_FACTOR==1
#if MATERIAL_HAS_SPECULAR_FACTOR==1
    dielectricSpecularF0 *= material.specularFactor;
    dielectricSpecularF90 = material.specularFactor;
#endif  // MATERIAL_HAS_SPECULAR_FACTOR==1
    pixel.f0 = baseColor.rgb * material.metallic + dielectricSpecularF0 * (1.0 - material.metallic);
    pixel.f90 = material.metallic + dielectricSpecularF90 * (1.0 - material.metallic);
#endif  // MATERIAL_HAS_SPECULAR_FACTOR==0 &&
        // MATERIAL_HAS_SPECULAR_COLOR_FACTOR==0
#else
    pixel.diffuseColor = baseColor.rgb;
    pixel.f0 = material.sheenColor;
#if MATERIAL_HAS_SUBSURFACE_COLOR==1
    pixel.subsurfaceColor = material.subsurfaceColor;
#endif  // MATERIAL_HAS_SUBSURFACE_COLOR==1
#endif  // SHADING_MODEL_SPECULAR_GLOSSINESS==1

#if SHADING_MODEL_CLOTH == 0 && SHADING_MODEL_SUBSURFACE == 0
#if MATERIAL_HAS_REFRACTION == 1
    const float airIor = 1.0;
#if MATERIAL_HAS_IOR == 0
    // [common case] ior is not set in the material, deduce it from F0
    float materialor = f0ToIor(pixel.f0.g);
#else
    float materialor = max(1.0, material.ior);
#endif                                  // MATERIAL_HAS_IOR==0
    pixel.etaIR = airIor / materialor;  // air -> material
    pixel.etaRI = materialor / airIor;  // material -> air
#if MATERIAL_HAS_TRANSMISSION == 1
    pixel.transmission = saturate(material.transmission);
#else
    pixel.transmission = 1.0;
#endif  // MATERIAL_HAS_TRANSMISSION==1
#if MATERIAL_HAS_ABSORPTION == 1
#if MATERIAL_HAS_THICKNESS == 1 || MATERIAL_HAS_MICRO_THICKNESS == 1
    pixel.absorption = max(vec3(0.0), material.absorption);
#else
    pixel.absorption = saturate(material.absorption);
#endif
#else
    pixel.absorption = vec3(0.0);
#endif
#if MATERIAL_HAS_THICKNESS == 1
    pixel.thickness = max(0.0, material.thickness);
#endif
#if MATERIAL_HAS_MICRO_THICKNESS == 1 && \
    (REFRACTION_TYPE == REFRACTION_TYPE_THIN)
    pixel.uThickness = max(0.0, material.microThickness);
#else
    pixel.uThickness = 0.0;
#endif  // MATERIAL_HAS_MICRO_THICKNESS==1 && (REFRACTION_TYPE ==
        // REFRACTION_TYPE_THIN)
#endif  // MATERIAL_HAS_REFRACTION==1
#endif  // SHADING_MODEL_CLOTH == 0 && SHADING_MODEL_SUBSURFACE == 0
#endif  // SHADING_MODEL_UNLIT==0
}

void getSheenPixelParams(const MaterialInputs material,
                         inout PixelParams pixel) {
#if MATERIAL_HAS_SHEEN_COLOR == 1 && SHADING_MODEL_CLOTH == 0 && \
    SHADING_MODEL_SUBSURFACE == 0
  pixel.sheenColor = material.sheenColor;

  float sheenPerceptualRoughness = material.sheenRoughness;
  sheenPerceptualRoughness =
      clamp(sheenPerceptualRoughness, MIN_PERCEPTUAL_ROUGHNESS, 1.0);

  // #if defined(GEOMETRIC_SPECULAR_AA)
  //   sheenPerceptualRoughness = normalFiltering(sheenPerceptualRoughness,
  //                                              getWorldGeometricNormalVector());
  // #endif

  pixel.sheenPerceptualRoughness = sheenPerceptualRoughness;
  pixel.sheenRoughness =
      perceptualRoughnessToRoughness(sheenPerceptualRoughness);
#endif
}

void getClearCoatPixelParams(const MaterialInputs material,
                             inout PixelParams pixel) {
#if MATERIAL_HAS_CLEAR_COAT == 1
  pixel.clearCoat = material.clearCoat;

  // Clamp the clear coat roughness to avoid divisions by 0
  float clearCoatPerceptualRoughness = material.clearCoatRoughness;
  clearCoatPerceptualRoughness =
      clamp(clearCoatPerceptualRoughness, MIN_PERCEPTUAL_ROUGHNESS, 1.0);

  // #if defined(GEOMETRIC_SPECULAR_AA)
  //   clearCoatPerceptualRoughness = normalFiltering(
  //       clearCoatPerceptualRoughness, getWorldGeometricNormalVector());
  // #endif

  pixel.clearCoatPerceptualRoughness = clearCoatPerceptualRoughness;
  pixel.clearCoatRoughness =
      perceptualRoughnessToRoughness(clearCoatPerceptualRoughness);
#endif
}

void getRoughnessPixelParams(const MaterialInputs material,
                             inout PixelParams pixel) {
#if SHADING_MODEL_UNLIT == 0
#if SHADING_MODEL_SPECULAR_GLOSSINESS == 1
  float perceptualRoughness =
      computeRoughnessFromGlossiness(material.glossiness);
#else
  float perceptualRoughness = material.roughness;
#endif

  // This is used by the refraction code and must be saved before we apply
  // specular AA
  pixel.perceptualRoughnessUnclamped = perceptualRoughness;

  // #if defined(GEOMETRIC_SPECULAR_AA)
  //   perceptualRoughness =
  //       normalFiltering(perceptualRoughness,
  //       getWorldGeometricNormalVector());
  // #endif

#if MATERIAL_HAS_CLEAR_COAT == 1 && MATERIAL_HAS_CLEAR_COAT_ROUGHNESS == 1
  // This is a hack but it will do: the base layer must be at least as rough
  // as the clear coat layer to take into account possible diffusion by the
  // top layer
  float basePerceptualRoughness =
      max(perceptualRoughness, pixel.clearCoatPerceptualRoughness);
  perceptualRoughness =
      mix(perceptualRoughness, basePerceptualRoughness, pixel.clearCoat);
#endif

  // Clamp the roughness to a minimum value to avoid divisions by 0 during
  // lighting
  pixel.perceptualRoughness =
      clamp(perceptualRoughness, MIN_PERCEPTUAL_ROUGHNESS, 1.0);
  // Remaps the roughness to a perceptually linear roughness (roughness^2)
  pixel.roughness = perceptualRoughnessToRoughness(pixel.perceptualRoughness);
#endif  // SHADING_MODEL_UNLIT==0
}

void getSubsurfacePixelParams(const MaterialInputs material,
                              inout PixelParams pixel) {
#if SHADING_MODEL_SUBSURFACE == 1
  pixel.subsurfacePower = material.subsurfacePower;
  pixel.subsurfaceColor = material.subsurfaceColor;
  pixel.thickness = saturate(material.thickness);
#endif
}

vec3 PrefilteredDFG_LUT(float lod, float NoV) {
  // coord = sqrt(linear_roughness), which is the mapping used by cmgen.
  return texture2D(sampler0_iblDFG, vec2(NoV, lod)).rgb;
}

//------------------------------------------------------------------------------
// IBL environment BRDF dispatch
//------------------------------------------------------------------------------

vec3 prefilteredDFG(float perceptualRoughness, float NoV) {
  // PrefilteredDFG_LUT() takes a LOD, which is sqrt(roughness) =
  // perceptualRoughness
  return PrefilteredDFG_LUT(perceptualRoughness, NoV);
}

void getEnergyCompensationPixelParams(inout PixelParams pixel) {
  // Pre-filtered DFG term used for image-based lighting
  pixel.dfg = prefilteredDFG(pixel.perceptualRoughness, shading_NoV);

#if SHADING_MODEL_CLOTH == 0
  // Energy compensation for multiple scattering in a microfacet model
  // See "Multiple-Scattering Microfacet BSDFs with the Smith Model"
  pixel.energyCompensation = 1.0 + pixel.f0 * (1.0 / pixel.dfg.y - 1.0);
#else
  pixel.energyCompensation = vec3(1.0);
#endif

#if SHADING_MODEL_CLOTH == 0
#if MATERIAL_HAS_SHEEN_COLOR == 1
  pixel.sheenDFG =
      prefilteredDFG(pixel.sheenPerceptualRoughness, shading_NoV).z;
  pixel.sheenScaling = 1.0 - max3(pixel.sheenColor) * pixel.sheenDFG;
#endif
#endif
}

void getAnisotropyPixelParams(const MaterialInputs material,
                              inout PixelParams pixel) {
#if MATERIAL_HAS_ANISOTROPY == 1
//   vec3 direction = material.anisotropyDirection;
//   pixel.anisotropy = material.anisotropy;
//   pixel.anisotropicT = normalize(shading_tangentToWorld * direction);
//   pixel.anisotropicB = normalize(cross(Normal, pixel.anisotropicT));
#endif
}

void getPixelParams(const MaterialInputs material, out PixelParams pixel) {
    getSpecularPixelParams(material, pixel);
    getCommonPixelParams(material, pixel);
    getSheenPixelParams(material, pixel);
    getClearCoatPixelParams(material, pixel);
    getRoughnessPixelParams(material, pixel);
    getSubsurfacePixelParams(material, pixel);
    // TODO: tangent vector loading is not implemented yet
    //  getAnisotropyPixelParams(material, pixel);
    getEnergyCompensationPixelParams(pixel);
}




void evaluateIBL(const MaterialInputs mat, const PixelParams pixel,
                 inout vec3 color) {
  vec3 Fr = vec3(0.0);
  SSAOInterpolationCache interpolationCache;
#if BLEND_MODE_OPAQUE == 1 || MATERIAL_HAS_REFLECTIONS == 1
  interpolationCache.uv = shading_normalizedViewportCoord;
#endif  // BLEND_MODE_OPAQUE==1|| MATERIAL_HAS_REFLECTIONS==1

#if MATERIAL_HAS_REFLECTIONS == 1
  vec4 ssrFr = vec4(0.0);
#endif  // MATERIAL_HAS_REFLECTIONS==1
  vec3 E = specularDFG(pixel);
  if(ssrFr.a <1.0){
    vec3 r = getReflectedVector(pixel, shading_normal);
    Fr = E * prefilteredRadiance(r,pixel.perceptualRoughness);
  }

  color.rgb += Fr;
}

vec4 evaluateLights(MaterialInputs material) {
  PixelParams pixel;
  getPixelParams(material, pixel);
  vec3 color = vec3(0.0);
  evaluateIBL(material, pixel, color);
  return vec4(color,1.0);
}

vec4 evaluateMaterial(MaterialInputs material) {
    vec4 color = evaluateLights(material);
    // evaluateLights(material);
    return color;
}

void prepareMaterial() {
  vec3 viewDir = normalize(cameraPos - FragPos);
  shading_NoV = clampNoV(dot(Normal, viewDir));
  shading_reflected = reflect(-viewDir, Normal);
  shading_normal = Normal;
  shading_view = (view*vec4(FragPos,1.0)).xyz;
  vec4 vertex_position = projection * view * vec4(FragPos, 1.0);
  shading_normalizedViewportCoord =
      vertex_position.xy * (0.5 / vertex_position.w) + 0.5;
}

void main() {
  MaterialInputs inputs;
  initMaterial(inputs);
  prepareMaterial();

  vec3 listDir = normalize(vec3(1.0, 1.0, 1.0));

  FragColor = vec4(vec3(FragPos.xyz), 1.0);
  FragColor = evaluateMaterial(inputs);
//   FragColor.xyz = inputs.normal;
}