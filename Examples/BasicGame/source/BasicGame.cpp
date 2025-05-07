#include "BasicGame.hpp"
#include <Pyramid/Graphics/GraphicsDevice.hpp>
#include <Pyramid/Graphics/Buffer/VertexBuffer.hpp>
#include <Pyramid/Graphics/Buffer/IndexBuffer.hpp>
#include <Pyramid/Graphics/Buffer/BufferLayout.hpp>
#include <vector>
#include <iostream> // For shader compilation error checking

// Simple vertex structure
struct Vertex
{
    float Position[3];
    float Color[3];
};

// GLSL Shader Sources
const std::string vertexShaderSrc = R"(
    #version 330 core
    layout (location = 0) in vec3 a_Position;
    layout (location = 1) in vec3 a_Color;

    out vec3 v_Color;

    void main()
    {
        v_Color = a_Color;
        gl_Position = vec4(a_Position, 1.0);
    }
)";

const std::string fragmentShaderSrc = R"(
    #version 330 core
    out vec4 FragColor;

    in vec3 v_Color;

    void main()
    {
        FragColor = vec4(v_Color, 1.0);
    }
)";

BasicGame::BasicGame()
    : Game(Pyramid::GraphicsAPI::OpenGL) // Request OpenGL
{
}

void BasicGame::onCreate()
{
    Game::onCreate(); // Important to call base for its setup

    Pyramid::IGraphicsDevice* device = GetGraphicsDevice();
    if (!device)
    {
        // PYRAMID_APP_ERROR("Graphics device is null!");
        std::cerr << "Error: Graphics device is null in BasicGame::onCreate!" << std::endl;
        return;
    }

    // 1. Create and compile shader
    m_shader = device->CreateShader();
    if (!m_shader)
    {
        std::cerr << "Error: Failed to create shader!" << std::endl;
        return;
    }
    if (!m_shader->Compile(vertexShaderSrc, fragmentShaderSrc))
    {
        std::cerr << "Error: Failed to compile shader!" << std::endl;
        // Shader compilation errors are already printed to cerr by OpenGLShader::Compile
        m_shader.reset(); // Release the failed shader
        return;
    }

    // 2. Define vertex and index data for a triangle
    Vertex vertices[] = {
        { { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } }, // Bottom-left, Red
        { {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } }, // Bottom-right, Green
        { {  0.0f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } }  // Top-center, Blue
    };

    Pyramid::u32 indices[] = { 0, 1, 2 };

    // 3. Create Vertex Buffer
    std::shared_ptr<Pyramid::IVertexBuffer> vbo = device->CreateVertexBuffer();
    vbo->SetData(vertices, sizeof(vertices));

    // 4. Create Index Buffer
    std::shared_ptr<Pyramid::IIndexBuffer> ibo = device->CreateIndexBuffer();
    ibo->SetData(indices, 3); // 3 indices

    // 5. Create Vertex Array Object (VAO)
    m_vertexArray = device->CreateVertexArray();

    // 6. Define Buffer Layout
    Pyramid::BufferLayout layout = {
        { Pyramid::ShaderDataType::Float3, "a_Position" },
        { Pyramid::ShaderDataType::Float3, "a_Color" }
    };

    // 7. Add Vertex Buffer (with layout) and Index Buffer to VAO
    m_vertexArray->AddVertexBuffer(vbo, layout);
    m_vertexArray->SetIndexBuffer(ibo);
}

void BasicGame::onUpdate(float deltaTime)
{
    // Call base class update (which by default clears the screen)
    Game::onUpdate(deltaTime);

    // Any other update logic for BasicGame can go here
}

void BasicGame::onRender()
{
    // Note: Screen clearing is handled by Game::onUpdate by default.
    // If you want custom clearing, do it here before drawing.

    if (m_shader && m_vertexArray)
    {
        m_shader->Bind();
        m_vertexArray->Bind();
        GetGraphicsDevice()->DrawIndexed(m_vertexArray->GetIndexBuffer()->GetCount()); // Use GetCount() from IBO
        m_vertexArray->Unbind();
        m_shader->Unbind();
    }

    // Call base class render (which by default presents the frame)
    Game::onRender();
}
