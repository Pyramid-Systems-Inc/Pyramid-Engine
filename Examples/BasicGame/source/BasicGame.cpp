#include "BasicGame.hpp"
#include <Pyramid/Graphics/GraphicsDevice.hpp>
#include <Pyramid/Graphics/Buffer/VertexBuffer.hpp>
#include <Pyramid/Graphics/Buffer/IndexBuffer.hpp>
#include <Pyramid/Graphics/Buffer/BufferLayout.hpp>
#include <Pyramid/Graphics/Texture.hpp> // Added
#include <Pyramid/Core/Log.hpp>         // Added
#include <vector>
// #include <iostream> // No longer needed directly
#include <cmath> // For sinf

// Updated vertex structure with texture coordinates
struct Vertex
{
    float Position[3];
    float Color[3]; // We can keep this, or remove if texture fully dictates color
    float TexCoord[2];
};

// GLSL Shader Sources - Updated for texturing
const std::string vertexShaderSrc = R"(
    #version 330 core
    layout (location = 0) in vec3 a_Position;
    layout (location = 1) in vec3 a_Color;      // Still taking color
    layout (location = 2) in vec2 a_TexCoord;

    out vec3 v_Color;
    out vec2 v_TexCoord;

    void main()
    {
        v_Color = a_Color;
        v_TexCoord = a_TexCoord;
        gl_Position = vec4(a_Position, 1.0);
    }
)";

const std::string fragmentShaderSrc = R"(
    #version 330 core
    out vec4 FragColor;

    in vec3 v_Color;
    in vec2 v_TexCoord;

    uniform sampler2D u_Texture;     // Texture sampler
    uniform vec3 u_ColorTint;        // Color tint uniform

    void main()
    {
        vec4 texColor = texture(u_Texture, v_TexCoord);
        FragColor = texColor * vec4(v_Color, 1.0) * vec4(u_ColorTint, 1.0); // Modulate texture, vertex color, and tint
    }
)";

BasicGame::BasicGame()
    : Game(Pyramid::GraphicsAPI::OpenGL)
{
}

void BasicGame::onCreate()
{
    Game::onCreate();

    Pyramid::IGraphicsDevice *device = GetGraphicsDevice();
    if (!device)
    {
        PYRAMID_LOG_ERROR("Graphics device is null in BasicGame::onCreate!"); // Changed
        return;
    }

    // 1. Create and compile shader
    m_shader = device->CreateShader();
    if (!m_shader || !m_shader->Compile(vertexShaderSrc, fragmentShaderSrc))
    {
        PYRAMID_LOG_ERROR("Failed to create or compile shader!"); // Changed
        m_shader.reset();
        return;
    }

    // 2. Load Texture using our custom Pyramid image loader
    // Now supports TGA and BMP formats through our custom implementation
    m_texture = device->CreateTexture2D("testtexture.tga");
    if (!m_texture || m_texture->GetRendererID() == 0) // Check if texture loading failed (OpenGLTexture sets ID to 0 on fail)
    {
        PYRAMID_LOG_ERROR("Failed to load texture 'testtexture.tga'!");
        // Continue without texture, or handle error more gracefully
        m_texture.reset();
    }

    // 3. Define vertex and index data for a triangle (now with UVs)
    // UVs: (0,0) bottom-left, (1,0) bottom-right, (0.5,1) top-center
    Vertex vertices[] = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // White, UV (0,0)
        {{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},  // White, UV (1,0)
        {{0.0f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.5f, 1.0f}}    // White, UV (0.5,1)
    };

    Pyramid::u32 indices[] = {0, 1, 2};

    // 4. Create Vertex Buffer
    std::shared_ptr<Pyramid::IVertexBuffer> vbo = device->CreateVertexBuffer();
    vbo->SetData(vertices, sizeof(vertices));

    // 5. Create Index Buffer
    std::shared_ptr<Pyramid::IIndexBuffer> ibo = device->CreateIndexBuffer();
    ibo->SetData(indices, 3);

    // 6. Create Vertex Array Object (VAO)
    m_vertexArray = device->CreateVertexArray();

    // 7. Define Buffer Layout (now includes TexCoord)
    Pyramid::BufferLayout layout = {
        {Pyramid::ShaderDataType::Float3, "a_Position"},
        {Pyramid::ShaderDataType::Float3, "a_Color"},
        {Pyramid::ShaderDataType::Float2, "a_TexCoord"}};

    // 8. Add Vertex Buffer (with layout) and Index Buffer to VAO
    m_vertexArray->AddVertexBuffer(vbo, layout);
    m_vertexArray->SetIndexBuffer(ibo);
}

void BasicGame::onUpdate(float deltaTime)
{
    Game::onUpdate(deltaTime);
    m_time += deltaTime;
}

void BasicGame::onRender()
{
    if (m_shader && m_vertexArray)
    {
        m_shader->Bind();

        // Set dynamic tint color
        float r = (sinf(m_time * 0.5f) + 1.0f) * 0.5f;
        float g = (sinf(m_time * 0.3f + 2.0f) + 1.0f) * 0.5f;
        float b = (sinf(m_time * 0.7f + 4.0f) + 1.0f) * 0.5f;
        m_shader->SetUniformFloat3("u_ColorTint", r, g, b);

        // Bind texture and set sampler uniform
        if (m_texture)
        {
            m_texture->Bind(0);                      // Bind to texture unit 0
            m_shader->SetUniformInt("u_Texture", 0); // Tell shader sampler u_Texture uses texture unit 0
        }
        else
        {
            // Fallback if texture failed to load: maybe set u_Texture to a default white texture or handle differently
            // For now, if no texture, it might sample from whatever was on unit 0, or default to black/white based on shader.
        }

        m_vertexArray->Bind();
        GetGraphicsDevice()->DrawIndexed(m_vertexArray->GetIndexBuffer()->GetCount());
        m_vertexArray->Unbind();
        // m_texture->Unbind(0); // Optional, usually not needed if next draw binds its own
        m_shader->Unbind();
    }

    Game::onRender();
}
