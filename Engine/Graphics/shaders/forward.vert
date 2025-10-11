#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

// Camera UBO (binding point 0)
layout(std140) uniform Camera {
    mat4 u_ViewMatrix;
    mat4 u_ProjectionMatrix;
    mat4 u_ViewProjection;
    vec4 u_CameraPosition;
    vec4 u_CameraDirection;
    float u_NearPlane;
    float u_FarPlane;
    float u_FOV;
    float u_AspectRatio;
};

// Per-object uniforms
uniform mat4 u_Model;
uniform mat4 u_NormalMatrix;

out vec3 v_WorldPos;
out vec3 v_Normal;
out vec2 v_TexCoord;

void main() {
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    v_WorldPos = worldPos.xyz;
    v_Normal = mat3(u_NormalMatrix) * a_Normal;
    v_TexCoord = a_TexCoord;
    
    gl_Position = u_ViewProjection * worldPos;
}