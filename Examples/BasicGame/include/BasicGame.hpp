#pragma once
#include <Pyramid/Core/Game.hpp>
#include <Pyramid/Graphics/Shader/Shader.hpp>
#include <Pyramid/Graphics/Buffer/VertexArray.hpp>
#include <Pyramid/Graphics/Buffer/UniformBuffer.hpp>
#include <Pyramid/Graphics/Texture.hpp>
#include <Pyramid/Math/Math.hpp>
#include <memory>
#include <vector>
#include <string>

class BasicGame : public Pyramid::Game
{
public:
    explicit BasicGame();
    virtual ~BasicGame() = default;

protected:
    void onCreate() override;
    void onUpdate(float deltaTime) override;
    void onRender() override;

private:
    void LoadTestTextures();
    void CreateSampleTextures();
    void SetupUniformBuffers();
    void UpdateUniformBuffers(float deltaTime);

    std::shared_ptr<Pyramid::IShader> m_shader;
    std::shared_ptr<Pyramid::IVertexArray> m_vertexArray;

    // OpenGL 4.6 Uniform Buffer Objects for efficient data transfer
    std::shared_ptr<Pyramid::IUniformBuffer> m_sceneUBO;
    std::shared_ptr<Pyramid::IUniformBuffer> m_materialUBO;

    // Scene uniform data structure (std140 layout)
    struct SceneUniforms
    {
        Pyramid::Math::Mat4 viewMatrix;
        Pyramid::Math::Mat4 projectionMatrix;
        Pyramid::Math::Vec4 cameraPosition;
        Pyramid::Math::Vec4 lightDirection;
        float time;
        float padding[3]; // Ensure 16-byte alignment
    };

    // Material uniform data structure (std140 layout)
    struct MaterialUniforms
    {
        Pyramid::Math::Vec4 baseColor;
        Pyramid::Math::Vec4 emissiveColor;
        float metallic;
        float roughness;
        float textureScale;
        float padding; // Ensure 16-byte alignment
    };

    // Multiple textures to showcase different image formats
    std::vector<std::shared_ptr<Pyramid::ITexture2D>> m_textures;
    std::vector<std::string> m_textureNames;
    std::vector<std::string> m_textureFormats;

    int m_currentTextureIndex = 0;
    float m_time = 0.0f;
    float m_textureSwapTimer = 0.0f;
    const float m_textureSwapInterval = 3.0f; // Switch texture every 3 seconds
};
