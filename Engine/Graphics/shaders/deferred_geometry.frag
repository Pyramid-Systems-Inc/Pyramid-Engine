#version 330 core

layout(location = 0) out vec4 gAlbedoMetallic;
layout(location = 1) out vec4 gNormalRoughness;
layout(location = 2) out vec4 gPositionAO;
layout(location = 3) out vec4 gEmissive;

in VS_OUT {
    vec3 WorldPos;
    vec3 Normal;
    vec2 TexCoord;
    mat3 TBN;
} fs_in;

// Material textures
uniform sampler2D u_AlbedoMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_MetallicRoughnessMap;
uniform sampler2D u_AOMap;
uniform sampler2D u_EmissiveMap;

// Material properties
uniform vec4 u_AlbedoColor;
uniform float u_Metallic;
uniform float u_Roughness;
uniform vec3 u_EmissiveColor;
uniform float u_EmissiveIntensity;

uniform bool u_HasAlbedoMap;
uniform bool u_HasNormalMap;
uniform bool u_HasMetallicRoughnessMap;
uniform bool u_HasAOMap;
uniform bool u_HasEmissiveMap;

void main() {
    // Albedo
    vec3 albedo = u_AlbedoColor.rgb;
    if (u_HasAlbedoMap) {
        albedo *= texture(u_AlbedoMap, fs_in.TexCoord).rgb;
    }
    
    // Normal mapping
    vec3 normal = fs_in.Normal;
    if (u_HasNormalMap) {
        vec3 normalMap = texture(u_NormalMap, fs_in.TexCoord).rgb;
        normalMap = normalize(normalMap * 2.0 - 1.0);
        normal = normalize(fs_in.TBN * normalMap);
    }
    
    // Metallic and Roughness
    float metallic = u_Metallic;
    float roughness = u_Roughness;
    if (u_HasMetallicRoughnessMap) {
        vec3 mr = texture(u_MetallicRoughnessMap, fs_in.TexCoord).rgb;
        metallic *= mr.b; // Blue channel = metallic
        roughness *= mr.g; // Green channel = roughness
    }
    
    // AO
    float ao = 1.0;
    if (u_HasAOMap) {
        ao = texture(u_AOMap, fs_in.TexCoord).r;
    }
    
    // Emissive
    vec3 emissive = u_EmissiveColor * u_EmissiveIntensity;
    if (u_HasEmissiveMap) {
        emissive *= texture(u_EmissiveMap, fs_in.TexCoord).rgb;
    }
    
    // Write to G-Buffer
    gAlbedoMetallic = vec4(albedo, metallic);
    gNormalRoughness = vec4(normal * 0.5 + 0.5, roughness); // Encode normal to [0,1]
    gPositionAO = vec4(fs_in.WorldPos, ao);
    gEmissive = vec4(emissive, 1.0); // Alpha can be used for flags
}