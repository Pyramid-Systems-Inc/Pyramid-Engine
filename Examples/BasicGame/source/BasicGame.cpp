#include "BasicGame.hpp"
#include <Pyramid/Graphics/GraphicsDevice.hpp>
#include <Pyramid/Graphics/Buffer/VertexBuffer.hpp>
#include <Pyramid/Graphics/Buffer/IndexBuffer.hpp>
#include <Pyramid/Graphics/Buffer/BufferLayout.hpp>
#include <Pyramid/Graphics/Texture.hpp>
#include <Pyramid/Util/Log.hpp>
#include <Pyramid/Util/Image.hpp> // Include our custom image loader
#include <vector>
#include <cmath>
#include <string>

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
    // Configure enhanced logging system
    Pyramid::Util::LoggerConfig logConfig;
    logConfig.enableConsole = true;
    logConfig.enableFile = true;
    logConfig.consoleLevel = Pyramid::Util::LogLevel::Info;
    logConfig.fileLevel = Pyramid::Util::LogLevel::Debug;
    logConfig.logFilePath = "pyramid_game.log";
    logConfig.enableTimestamp = true;
    logConfig.enableSourceLocation = true;
    logConfig.enableThreadId = false; // Keep console output clean
    PYRAMID_CONFIGURE_LOGGER(logConfig);

    PYRAMID_LOG_INFO("BasicGame starting up with enhanced logging system");

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

    // 2. Load multiple textures to showcase our custom Pyramid image loader
    // Supports TGA, BMP, and PNG formats through our custom implementation
    LoadTestTextures();

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

void BasicGame::LoadTestTextures()
{
    Pyramid::IGraphicsDevice *device = GetGraphicsDevice();
    if (!device)
    {
        PYRAMID_LOG_ERROR("Graphics device is null in LoadTestTextures!");
        return;
    }

    // First, try to create sample textures programmatically
    CreateSampleTextures();

    // List of test texture files to try loading
    std::vector<std::string> testFiles = {
        "test.tga",
        "test.bmp",
        "test.png",
        "sample.tga",
        "sample.bmp",
        "sample.png",
        "texture.tga",
        "texture.bmp",
        "texture.png"};

    std::vector<std::string> formats = {
        "TGA", "BMP", "PNG", "TGA", "BMP", "PNG", "TGA", "BMP", "PNG"};

    // Try to load each test file
    for (size_t i = 0; i < testFiles.size(); ++i)
    {
        auto texture = device->CreateTexture2D(testFiles[i]);
        if (texture && texture->GetRendererID() != 0)
        {
            m_textures.push_back(texture);
            m_textureNames.push_back(testFiles[i]);
            m_textureFormats.push_back(formats[i]);
            PYRAMID_LOG_INFO("Successfully loaded texture: ", testFiles[i], " (", formats[i], ")");
        }
        else
        {
            PYRAMID_LOG_WARN("Failed to load texture: ", testFiles[i]);
        }
    }

    if (m_textures.empty())
    {
        PYRAMID_LOG_WARN("No textures loaded! The demo will run without textures.");
        PYRAMID_LOG_INFO("To see textures, place test.tga, test.bmp, or test.png files in the working directory.");
    }
    else
    {
        PYRAMID_LOG_INFO("Loaded ", m_textures.size(), " textures successfully!");
        PYRAMID_LOG_INFO("Textures will cycle every ", m_textureSwapInterval, " seconds.");
    }
}

void BasicGame::CreateSampleTextures()
{
    // Create simple procedural textures to demonstrate our image loader
    Pyramid::IGraphicsDevice *device = GetGraphicsDevice();
    if (!device)
        return;

    // Create a simple 64x64 checkerboard pattern and save as different formats
    const int size = 64;
    std::vector<uint8_t> pixels(size * size * 3);

    // Generate checkerboard pattern
    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            int index = (y * size + x) * 3;
            bool isWhite = ((x / 8) + (y / 8)) % 2 == 0;

            if (isWhite)
            {
                pixels[index] = 255;     // R
                pixels[index + 1] = 255; // G
                pixels[index + 2] = 255; // B
            }
            else
            {
                pixels[index] = 64;      // R
                pixels[index + 1] = 128; // G
                pixels[index + 2] = 255; // B
            }
        }
    }

    // Try to save and load the procedural texture
    // Note: This would require implementing image saving, which we haven't done yet
    // For now, we'll just log that we tried to create sample textures
    PYRAMID_LOG_INFO("Generated procedural checkerboard pattern (", size, "x", size, ")");
    PYRAMID_LOG_INFO("Image saving not yet implemented - place test images manually");
}

void BasicGame::onUpdate(float deltaTime)
{
    Game::onUpdate(deltaTime);
    m_time += deltaTime;
    m_textureSwapTimer += deltaTime;

    // Cycle through textures
    if (!m_textures.empty() && m_textureSwapTimer >= m_textureSwapInterval)
    {
        m_currentTextureIndex = (m_currentTextureIndex + 1) % m_textures.size();
        m_textureSwapTimer = 0.0f;

        PYRAMID_LOG_INFO("Switched to texture: ", m_textureNames[m_currentTextureIndex],
                         " (", m_textureFormats[m_currentTextureIndex], ")");
    }
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

        // Bind current texture and set sampler uniform
        if (!m_textures.empty() && m_currentTextureIndex < m_textures.size())
        {
            auto currentTexture = m_textures[m_currentTextureIndex];
            if (currentTexture)
            {
                currentTexture->Bind(0);                 // Bind to texture unit 0
                m_shader->SetUniformInt("u_Texture", 0); // Tell shader sampler u_Texture uses texture unit 0
            }
        }
        else
        {
            // No textures loaded - the shader will use default sampling
            // (Only log this once to avoid spam)
        }

        m_vertexArray->Bind();
        GetGraphicsDevice()->DrawIndexed(m_vertexArray->GetIndexBuffer()->GetCount());
        m_vertexArray->Unbind();
        // m_texture->Unbind(0); // Optional, usually not needed if next draw binds its own
        m_shader->Unbind();
    }

    Game::onRender();
}
