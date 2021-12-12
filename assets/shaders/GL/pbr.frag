#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

layout(set =0,binding = 0) uniform RenderState {
    mat4 world;
    mat4 view;
    mat4 proj;
    vec3 cameraPosWS;
    float lerpUserValues;
    float userMetalness;
    float userRoughness;
} ubo;


layout(set=1,binding = 0) uniform sampler2D albedoSampler;
layout(set=1,binding = 1) uniform sampler2D normalSampler;
layout(set=1,binding = 2) uniform sampler2D aoSampler;
layout(set=1,binding = 3) uniform sampler2D shadingSampler;
layout(set=1,binding = 4) uniform sampler2D emissionSampler;
layout(set=1,binding = 5) uniform samplerCube environmentSampler;
layout(set=1,binding = 6) uniform samplerCube diffuseIrradianceSampler;
layout(set = 1, binding = 7) uniform sampler2D bakedBRDFSampler;



const float PI =  3.141592653589798979f;
const float PI2 = 6.283185307179586477f;
const float iPI = 0.318309886183790672f;

struct Surface
{
    vec3 light;
    vec3 view;
    vec3 normal;
    vec3 halfVector;
    float dotNH;
    float dotNL;
    float dotNV;
    float dotHV;
};

float sqr(float a)
{
    return a * a;
}

float lerp(float a, float b, float t)
{
    return a * (1.0f - t) + b * t;
}

vec3 lerp(vec3 a, vec3 b, float t)
{
    return a * (1.0f - t) + b * t;
}

vec3 lerp(vec3 a, vec3 b, vec3 t)
{
    return a * (1.0f - t) + b * t;
}

float saturate(float v)
{
    return clamp(v, 0.0f, 1.0f);
}

vec3 saturate(vec3 v)
{
    vec3 ret;
    ret.x = clamp(v.x, 0.0f, 1.0f);
    ret.y = clamp(v.y, 0.0f, 1.0f);
    ret.z = clamp(v.z, 0.0f, 1.0f);
    return ret;
}

vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i) / float(N), float(bitfieldReverse(i)) * 2.3283064365386963e-10);
}

vec3 ImportanceSamplingGGX(vec2 Xi, vec3 normal, float roughness)
{
    float alpha = sqr(roughness * roughness);
    float alpha2 = saturate(sqr(alpha));

    float phi = PI2 * Xi.x;
    float cosTheta = sqrt((1.0f - Xi.y) / (1.0f + (alpha2 - 1.0f) * Xi.y));
    float sinTheta = sqrt(1.0f - sqr(cosTheta));

    // from spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // from tangent-space vector to world-space sample vector
    vec3 up        = abs(normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, normal));
    vec3 bitangent = cross(normal, tangent);

    vec3 sampleVec = tangent * H.x + bitangent * H.y + normal * H.z;
    return normalize(sampleVec);
}

float D_GGX(Surface surface, float roughness)
{
    float alpha2 = saturate(sqr(roughness * roughness));

    return alpha2 / (PI * sqr(1.0f + surface.dotNH * surface.dotNH * (alpha2 - 1.0f)));
}

float G_SmithGGX_Normalized(Surface surface, float roughness)
{
    float alpha2 = saturate(sqr(roughness * roughness));

    float ggx_NV = surface.dotNV + sqrt(alpha2 + (1.0f - alpha2) * surface.dotNV * surface.dotNV);
    float ggx_NL = surface.dotNL + sqrt(alpha2 + (1.0f - alpha2) * surface.dotNL * surface.dotNL);

    return 1.0f / (ggx_NV * ggx_NL);
}

float G_SmithGGX(Surface surface, float roughness)
{
    return 4.0f * surface.dotNV * surface.dotNL * G_SmithGGX_Normalized(surface, roughness);
}

float F_Shlick(Surface surface)
{
    return pow(1.0f - surface.dotHV, 5.0f);
}

vec3 F_Shlick(Surface surface, vec3 f0)
{
    return f0 + (vec3(1.0f, 1.0f, 1.0f) - f0) * pow(1.0f - surface.dotHV, 5.0f);
}

vec3 F_Shlick(float cosTheta, vec3 f0, float roughness)
{
    return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(1.0f - cosTheta, 5.0f);
}


layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragTangentWS;
layout(location = 3) in vec3 fragBinormalWS;
layout(location = 4) in vec3 fragNormalWS;
layout(location = 5) in vec3 fragPositionWS;

layout(location = 0) out vec4 outColor;


void main()
{

    vec3 lightPosWS = ubo.cameraPosWS;
    vec3 lightDirWS = normalize(lightPosWS - fragPositionWS);
    vec3 cameraDirWS = normalize(ubo.cameraPosWS - fragPositionWS);

    vec3 normal = texture(normalSampler, fragTexCoord).xyz * 2.0f - vec3(1.0f);

    mat3 m;
    m[0] = normalize(fragTangentWS);
    m[1] = normalize(-fragBinormalWS);
    m[2] = normalize(fragNormalWS);

    Surface surface;
    surface.light = lightDirWS;
    surface.view = cameraDirWS;
    surface.normal = normalize(m * normal);
    surface.halfVector = normalize(lightDirWS + cameraDirWS);
    surface.dotNH = max(0.0f, dot(surface.normal, surface.halfVector));
    surface.dotNL = max(0.0f, dot(surface.normal, surface.light));
    surface.dotNV = max(0.0f, dot(surface.normal, surface.view));
    surface.dotHV = max(0.0f, dot(surface.halfVector, surface.view));

    MicrofacetMaterial microfacet_material;
    microfacet_material.albedo = texture(albedoSampler, fragTexCoord).rgb;
    microfacet_material.albedo = pow(microfacet_material.albedo, vec3(2.2f));
    microfacet_material.roughness = texture(shadingSampler, fragTexCoord).g;
    microfacet_material.metalness = texture(shadingSampler, fragTexCoord).b;
    microfacet_material.ao = texture(aoSampler, fragTexCoord).r;
    microfacet_material.ao = pow(microfacet_material.ao, 2.2f);

    microfacet_material.albedo = lerp(microfacet_material.albedo, vec3(0.5f, 0.5f, 0.5f), ubo.lerpUserValues);
    microfacet_material.roughness = lerp(microfacet_material.roughness, ubo.userRoughness, ubo.lerpUserValues);
    microfacet_material.metalness = lerp(microfacet_material.metalness, ubo.userMetalness, ubo.lerpUserValues);

    microfacet_material.f0 = lerp(vec3(0.04f), microfacet_material.albedo, microfacet_material.metalness);

    // Direct light
    float attenuation = 1.0f / dot(lightPosWS - fragPositionWS, lightPosWS - fragPositionWS);

    vec3 light = DirectBRDF(surface, microfacet_material) * attenuation * 2.0f * surface.dotNL;

    // Ambient light (diffuse & specular IBL)
    vec3 ambient = IBLBRDF(surface, microfacet_material);

    // Result
    vec3 color = vec3(0.0f);
    color += ambient;
    color += light;

    // TODO: move to separate pass
    // Tonemapping + gamma correction
    // color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0f / 2.2f));

    outColor = vec4(color, 1.0f);
}
