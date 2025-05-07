#pragma once
#include <Pyramid/Core/Game.hpp>
#include <Pyramid/Graphics/Shader/Shader.hpp>
#include <Pyramid/Graphics/Buffer/VertexArray.hpp>
#include <memory> // For std::shared_ptr

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
    std::shared_ptr<Pyramid::IShader> m_shader;
    std::shared_ptr<Pyramid::IVertexArray> m_vertexArray;
};
