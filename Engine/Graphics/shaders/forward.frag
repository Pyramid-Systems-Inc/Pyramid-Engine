#version 330 core

in vec3 v_WorldPos;
in vec3 v_Normal;
in vec2 v_TexCoord;

uniform sampler2D u_AlbedoMap;
uniform vec4 u_AlbedoColor;
uniform int u_HasAlbedoMap;

out vec4 FragColor;

void main() {
    vec3 normal = normalize(v_Normal);
    
    // Sample albedo
    vec4 albedo = u_AlbedoColor;
    if (u_HasAlbedoMap == 1) {
        albedo *= texture(u_AlbedoMap, v_TexCoord);
    }
    
    // Simple lighting (directional light from above)
    vec3 lightDir = normalize(vec3(0.3, 1.0, 0.5));
    float diffuse = max(dot(normal, lightDir), 0.0);
    
    vec3 ambient = vec3(0.2);
    vec3 lighting = ambient + diffuse * vec3(1.0);
    
    FragColor = vec4(albedo.rgb * lighting, albedo.a);
}