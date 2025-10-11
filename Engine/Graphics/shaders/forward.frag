#version 330 core

in vec3 v_WorldPos;
in vec3 v_Normal;
in vec2 v_TexCoord;

out vec4 FragColor;

// Material properties
uniform sampler2D u_AlbedoMap;
uniform vec4 u_AlbedoColor;
uniform int u_HasAlbedoMap;

// Shadow mapping
uniform sampler2DArray u_ShadowMaps; // Cascaded shadow maps
uniform mat4 u_LightSpaceMatrices[4]; // Max 4 cascades
uniform float u_CascadeSplits[5]; // Near + cascade splits + far
uniform int u_CascadeCount;
uniform vec3 u_LightDirection;
uniform float u_ShadowBias;

// Camera UBO
layout(std140) uniform Camera {
    mat4 u_ViewProjection;
    mat4 u_View;
    mat4 u_Projection;
    vec4 u_CameraPosition;
};

// Calculate shadow with PCF (Percentage Closer Filtering)
float CalculateShadow(vec3 worldPos, vec3 normal) {
    // Determine cascade index based on view space depth
    vec4 viewPos = u_View * vec4(worldPos, 1.0);
    float depth = abs(viewPos.z);
    
    int cascadeIndex = 0;
    for (int i = 0; i < u_CascadeCount; i++) {
        if (depth < u_CascadeSplits[i + 1]) {
            cascadeIndex = i;
            break;
        }
    }
    
    // Transform to light space
    vec4 lightSpacePos = u_LightSpaceMatrices[cascadeIndex] * vec4(worldPos, 1.0);
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5; // Transform to [0,1]
    
    // Outside shadow map bounds = no shadow
    if (projCoords.z > 1.0) return 0.0;
    
    // Bias to reduce shadow acne
    float bias = max(u_ShadowBias * (1.0 - dot(normal, -u_LightDirection)), u_ShadowBias * 0.1);
    
    // PCF (Percentage Closer Filtering)
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(u_ShadowMaps, 0).xy);
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float pcfDepth = texture(u_ShadowMaps, vec3(projCoords.xy + vec2(x, y) * texelSize, cascadeIndex)).r;
            shadow += (projCoords.z - bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    
    return shadow;
}

void main() {
    vec3 normal = normalize(v_Normal);
    
    // Sample albedo
    vec4 albedo = u_AlbedoColor;
    if (u_HasAlbedoMap == 1) {
        albedo *= texture(u_AlbedoMap, v_TexCoord);
    }
    
    // Directional light
    vec3 lightDir = normalize(-u_LightDirection);
    float diffuse = max(dot(normal, lightDir), 0.0);
    
    // Calculate shadow (if shadow maps are available)
    float shadow = 0.0;
    if (u_CascadeCount > 0) {
        shadow = CalculateShadow(v_WorldPos, normal);
    }
    
    // Lighting
    vec3 ambient = vec3(0.2);
    vec3 lighting = ambient + (1.0 - shadow) * diffuse * vec3(1.0);
    
    FragColor = vec4(albedo.rgb * lighting, albedo.a);
}