#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
	mat4 world;
	mat4 view;
	mat4 proj;
	vec3 cameraPos;
} ubo;

layout(binding = 1) uniform sampler2D albedoSampler;
layout(binding = 2) uniform sampler2D normalSampler;
layout(binding = 3) uniform sampler2D aoSampler;
layout(binding = 4) uniform sampler2D shadingSampler;
layout(binding = 5) uniform sampler2D emissionSampler;
layout(binding = 5) uniform sampler2D hdrSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragTangentWS;
layout(location = 3) in vec3 fragBinormalWS;
layout(location = 4) in vec3 fragNormalWS;
layout(location = 5) in vec3 fragPositionWS;

layout(location = 0) out vec4 outColor;

const float PI = 3.141592653589798979f;
const float iPI = 0.31830988618379f;

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

//////////////// Vanilla world

struct VanillaMaterial
{
	float shininess;
	vec3 color;
};

vec3 DiffuseLambertBRDF(Surface surface, VanillaMaterial material)
{
	float dotNL = max(dot(surface.normal, surface.light), 0.0f);
	return vec3(1.0f, 1.0f, 1.0f) * dotNL;
}

vec3 DiffuseHalfLambertBRDF(Surface surface, VanillaMaterial material)
{
	float dotNL = max(dot(surface.normal, surface.light), 0.0f);
	float halfLambert = (dotNL + 1) * 0.5f;
	return vec3(1.0f, 1.0f, 1.0f) * halfLambert * halfLambert;
}

vec3 SpecularBlinnPhongBRDF(Surface surface, VanillaMaterial material)
{
	vec3 h = normalize(surface.light + surface.view);
	float dotNH = max(dot(surface.normal, h), 0.0f);
	return vec3(1.0f, 1.0f, 1.0f) * pow(dotNH, material.shininess);
}

vec3 SpecularPhongBRDF(Surface surface, VanillaMaterial material)
{
	vec3 r = reflect(-surface.light, surface.normal);
	float dotRV = max(dot(r, surface.view), 0.0f);
	return vec3(1.0f, 1.0f, 1.0f) * pow(dotRV, material.shininess / 4.0f);
}

vec3 VanillaBRDF(Surface surface, VanillaMaterial material)
{
	vec3 diffuse = DiffuseHalfLambertBRDF(surface, material);
	vec3 specular = SpecularBlinnPhongBRDF(surface, material);

	return diffuse * material.color + specular;
}

//////////////// Microfacet world

struct MicrofacetMaterial
{
	vec3 albedo;
	float roughness;
	float roughness_pow;
	float metalness;

};

float D_GGX(Surface surface, float roughness)
{
	float alpha2 = sqr(roughness * roughness);

	return alpha2 * iPI / sqr(1.0f + surface.dotNH *surface.dotNH * (alpha2 - 1.0f));
}

float G_SmithGGX_Normalized(Surface surface, float roughness)
{
	float alpha2 = sqr(roughness * roughness);

	float ggx_NV = surface.dotNV + sqrt(alpha2 + (1.0f - alpha2) *surface.dotNV * surface.dotNV);
	float ggx_NL = surface.dotNL + sqrt(alpha2 + (1.0f - alpha2) *surface.dotNL * surface.dotNL);

	return 1.0/(ggx_NV * ggx_NL);
}

vec3 F_Shlick(Surface surface, vec3 f0)
{
	return f0 + (vec3(1.0f, 1.0f, 1.0f) - f0) * pow(1.0f -surface.dotHV, 5);
}


vec3 MicrofacetBRDF(Surface surface, MicrofacetMaterial material)
{

	vec3 f0_dielectric = vec3(0.04f);
	vec3 f0 = lerp(f0_dielectric,material.albedo,material.metalness);

	float D = D_GGX(surface, material.roughness);
	float G_Normalized = G_SmithGGX_Normalized(surface, material.roughness);
	vec3  F = F_Shlick(surface, f0_dielectric);

	vec3 specluar_reflection =D * G_Normalized * F ;
	vec3 diffuse_reflection =lerp(vec3(1.f) - F,vec3(0.f),material.metalness);

	return (diffuse_reflection* material.albedo * iPI + specluar_reflection);
}

void main() {
	vec3 lightPos = ubo.cameraPos;
	vec3 lightDirWS = normalize(lightPos - fragPositionWS);
	vec3 cameraDirWS = normalize(ubo.cameraPos - fragPositionWS);

	vec3 normal = texture(normalSampler, fragTexCoord).xyz * 2.0f - vec3(1.0f, 1.0f, 1.0f);

	mat3 m;
	m[0] = normalize(fragTangentWS);
	m[1] = normalize(fragBinormalWS);
	m[2] = normalize(fragNormalWS);

	Surface surface;
	surface.light = lightDirWS;
	surface.view = cameraDirWS;
	surface.normal = normalize(m * normal);
	surface.halfVector = normalize(lightDirWS + cameraDirWS);
	surface.dotNL = max(0.0f,dot(surface.normal,surface.light));
	surface.dotNV = max(0.0f,dot(surface.normal,surface.view));
	surface.dotNH = max(0.0f,dot(surface.normal,surface.halfVector));
	surface.dotHV = max(0.f,dot(surface.halfVector,surface.view));

//	if (true) {
	MicrofacetMaterial microfacet_material;
	microfacet_material.albedo = texture(albedoSampler, fragTexCoord).rgb;
	microfacet_material.roughness = texture(shadingSampler, fragTexCoord).g;
	microfacet_material.metalness = texture(shadingSampler, fragTexCoord).b;
	microfacet_material.roughness_pow= sqr( microfacet_material.roughness * microfacet_material.roughness);

	//Diretc  light
	float dotNL = max(dot(surface.normal,surface.light),0.0);
	float attenuation = 1.f / dot(lightPos -fragPositionWS,lightPos -fragPositionWS);
	vec3 light = MicrofacetBRDF(surface, microfacet_material)* attenuation *2.0* surface.dotNL;

	//ambient light
	vec3 ambient =  microfacet_material.albedo * vec3(0.03f) * texture(aoSampler,fragTexCoord).r;

	//Result
	vec3 color = ambient;
	color += light;
	color +=texture(emissionSampler,fragTexCoord).rgb;

	//Gramma Correction
	color = color / (color+ vec3(1.0));
	color = pow(color,vec3(1.0/2.2));
	outColor = vec4(color, 1.0f);
//	}
//	else
//	{
//		VanillaMaterial vanilla_material;
//		vanilla_material.shininess = 10.0f;
//		vanilla_material.color = texture(albedoSampler, fragTexCoord).rgb;
//
//		vec3 vanilla_brdf = VanillaBRDF(surface, vanilla_material);
//		outColor = vec4(vanilla_brdf, 1.0f);
//	}

//	outColor *= texture(aoSampler, fragTexCoord);
//	outColor += texture(emissionSampler, fragTexCoord);
}
