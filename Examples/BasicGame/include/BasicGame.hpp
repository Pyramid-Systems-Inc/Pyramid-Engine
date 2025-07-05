#pragma once
#include <Pyramid/Core/Game.hpp>
#include <Pyramid/Graphics/Shader/Shader.hpp>
#include <Pyramid/Graphics/Buffer/VertexArray.hpp>
#include <Pyramid/Graphics/Texture.hpp>
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
    std::shared_ptr<Pyramid::IShader> m_shader;
    std::shared_ptr<Pyramid::IVertexArray> m_vertexArray;

    // Multiple textures to showcase different image formats
    std::vector<std::shared_ptr<Pyramid::ITexture2D>> m_textures;
    std::vector<std::string> m_textureNames;
    std::vector<std::string> m_textureFormats;

    int m_currentTextureIndex = 0;
    float m_time = 0.0f;
    float m_textureSwapTimer = 0.0f;
    const float m_textureSwapInterval = 3.0f; // Switch texture every 3 seconds
};
