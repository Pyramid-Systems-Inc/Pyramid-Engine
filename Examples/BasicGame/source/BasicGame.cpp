#include "BasicGame.hpp"
#include <Pyramid/Graphics/GraphicsDevice.hpp>
#include <Pyramid/Graphics/Buffer/VertexBuffer.hpp>
#include <Pyramid/Graphics/Buffer/IndexBuffer.hpp>
#include <Pyramid/Graphics/Buffer/BufferLayout.hpp>
#include <Pyramid/Graphics/Buffer/UniformBuffer.hpp>
#include <Pyramid/Graphics/Texture.hpp>
#include <Pyramid/Util/Log.hpp>
#include <Pyramid/Util/Image.hpp> // Include our custom image loader
#include <vector>
#include <cmath>
#include <string>
#include <cstring> // For memset

// Updated vertex structure with texture coordinates
struct Vertex
{
    float Position[3];
    float Color[3]; // We can keep this, or remove if texture fully dictates color
    float TexCoord[2];
};

// GLSL Shader Sources - Updated for OpenGL 4.6 with Uniform Buffer Objects
const std::string vertexShaderSrc = R"(
    #version 460 core
    layout (location = 0) in vec3 a_Position;
    layout (location = 1) in vec3 a_Color;
    layout (location = 2) in vec2 a_TexCoord;

    // Uniform Buffer Objects (std140 layout)
    layout(std140, binding = 0) uniform SceneData {
        mat4 u_ViewMatrix;
        mat4 u_ProjectionMatrix;
        vec4 u_CameraPosition;
        vec4 u_LightDirection;
        float u_Time;
    };

    out vec3 v_Color;
    out vec2 v_TexCoord;
    out vec3 v_WorldPos;

    void main()
    {
        v_Color = a_Color;
        v_TexCoord = a_TexCoord;
        v_WorldPos = a_Position;

        // Apply view and projection transformations
        vec4 worldPos = vec4(a_Position, 1.0);
        vec4 viewPos = u_ViewMatrix * worldPos;
        gl_Position = u_ProjectionMatrix * viewPos;
    }
)";

const std::string fragmentShaderSrc = R"(
    #version 460 core
    out vec4 FragColor;

    in vec3 v_Color;
    in vec2 v_TexCoord;
    in vec3 v_WorldPos;

    // Uniform Buffer Objects (std140 layout)
    layout(std140, binding = 0) uniform SceneData {
        mat4 u_ViewMatrix;
        mat4 u_ProjectionMatrix;
        vec4 u_CameraPosition;
        vec4 u_LightDirection;
        float u_Time;
    };

    layout(std140, binding = 1) uniform MaterialData {
        vec4 u_BaseColor;
        vec4 u_EmissiveColor;
        float u_Metallic;
        float u_Roughness;
        float u_TextureScale;
    };

    uniform sampler2D u_Texture;

    void main()
    {
        // Sample texture with animated UV coordinates
        vec2 animatedUV = v_TexCoord * u_TextureScale + sin(u_Time * 0.5) * 0.1;
        vec4 texColor = texture(u_Texture, animatedUV);

        // Simple lighting calculation using uniform buffer data
        vec3 lightDir = normalize(u_LightDirection.xyz);
        vec3 normal = vec3(0.0, 0.0, 1.0); // Simple flat normal for triangle
        float NdotL = max(dot(normal, lightDir), 0.0);

        // Combine all color sources
        vec3 diffuse = texColor.rgb * u_BaseColor.rgb * NdotL;
        vec3 emissive = u_EmissiveColor.rgb * sin(u_Time) * 0.5 + 0.5;
        vec3 finalColor = (diffuse + emissive) * v_Color;

        FragColor = vec4(finalColor, texColor.a * u_BaseColor.a);
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

    PYRAMID_LOG_INFO("Shader compiled successfully with OpenGL 4.6 uniform blocks");

    // 2. Setup OpenGL 4.6 Uniform Buffer Objects
    SetupUniformBuffers();

    // 3. Load multiple textures to showcase our custom Pyramid image loader
    // Supports TGA, BMP, and PNG formats through our custom implementation
    LoadTestTextures();

    // 4. Define vertex and index data for a triangle (now with UVs)
    // UVs: (0,0) bottom-left, (1,0) bottom-right, (0.5,1) top-center
    Vertex vertices[] = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // White, UV (0,0)
        {{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},  // White, UV (1,0)
        {{0.0f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.5f, 1.0f}}    // White, UV (0.5,1)
    };

    Pyramid::u32 indices[] = {0, 1, 2};

    // 5. Create Vertex Buffer
    std::shared_ptr<Pyramid::IVertexBuffer> vbo = device->CreateVertexBuffer();
    vbo->SetData(vertices, sizeof(vertices));

    // 6. Create Index Buffer
    std::shared_ptr<Pyramid::IIndexBuffer> ibo = device->CreateIndexBuffer();
    ibo->SetData(indices, 3);

    // 7. Create Vertex Array Object (VAO)
    m_vertexArray = device->CreateVertexArray();

    // 8. Define Buffer Layout (now includes TexCoord)
    Pyramid::BufferLayout layout = {
        {Pyramid::ShaderDataType::Float3, "a_Position"},
        {Pyramid::ShaderDataType::Float3, "a_Color"},
        {Pyramid::ShaderDataType::Float2, "a_TexCoord"}};

    // 9. Add Vertex Buffer (with layout) and Index Buffer to VAO
    m_vertexArray->AddVertexBuffer(vbo, layout);
    m_vertexArray->SetIndexBuffer(ibo);

    PYRAMID_LOG_INFO("BasicGame initialized with OpenGL 4.6 Uniform Buffer Objects!");
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

void BasicGame::SetupUniformBuffers()
{
    Pyramid::IGraphicsDevice *device = GetGraphicsDevice();
    if (!device)
    {
        PYRAMID_LOG_ERROR("Graphics device is null in SetupUniformBuffers!");
        return;
    }

    // Create Scene Uniform Buffer (binding point 0)
    m_sceneUBO = device->CreateUniformBuffer(sizeof(SceneUniforms));
    if (!m_sceneUBO)
    {
        PYRAMID_LOG_ERROR("Failed to create scene uniform buffer!");
        return;
    }

    // Create Material Uniform Buffer (binding point 1)
    m_materialUBO = device->CreateUniformBuffer(sizeof(MaterialUniforms));
    if (!m_materialUBO)
    {
        PYRAMID_LOG_ERROR("Failed to create material uniform buffer!");
        return;
    }

    // Initialize uniform buffer data
    SceneUniforms sceneData = {};

    // Identity matrix for view
    memset(sceneData.viewMatrix, 0, sizeof(sceneData.viewMatrix));
    sceneData.viewMatrix[0] = sceneData.viewMatrix[5] = sceneData.viewMatrix[10] = sceneData.viewMatrix[15] = 1.0f;

    // Simple perspective projection matrix
    memset(sceneData.projectionMatrix, 0, sizeof(sceneData.projectionMatrix));
    float fov = 45.0f * 3.14159f / 180.0f;
    float aspect = 800.0f / 600.0f;
    float near = 0.1f, far = 100.0f;
    float f = 1.0f / tanf(fov * 0.5f);
    sceneData.projectionMatrix[0] = f / aspect;
    sceneData.projectionMatrix[5] = f;
    sceneData.projectionMatrix[10] = (far + near) / (near - far);
    sceneData.projectionMatrix[11] = -1.0f;
    sceneData.projectionMatrix[14] = (2.0f * far * near) / (near - far);

    // Camera position
    sceneData.cameraPosition[0] = 0.0f;
    sceneData.cameraPosition[1] = 0.0f;
    sceneData.cameraPosition[2] = 3.0f;
    sceneData.cameraPosition[3] = 1.0f;

    // Light direction
    sceneData.lightDirection[0] = 0.5f;
    sceneData.lightDirection[1] = -1.0f;
    sceneData.lightDirection[2] = 0.5f;
    sceneData.lightDirection[3] = 0.0f;

    sceneData.time = 0.0f;

    MaterialUniforms materialData = {};
    materialData.baseColor[0] = materialData.baseColor[1] = materialData.baseColor[2] = materialData.baseColor[3] = 1.0f;
    materialData.emissiveColor[0] = 0.2f;
    materialData.emissiveColor[1] = 0.4f;
    materialData.emissiveColor[2] = 0.8f;
    materialData.emissiveColor[3] = 1.0f;
    materialData.metallic = 0.0f;
    materialData.roughness = 0.5f;
    materialData.textureScale = 1.0f;

    // Upload initial data to uniform buffers
    m_sceneUBO->UpdateData(&sceneData, sizeof(SceneUniforms));
    m_materialUBO->UpdateData(&materialData, sizeof(MaterialUniforms));

    PYRAMID_LOG_INFO("Uniform buffers created and initialized successfully!");
    PYRAMID_LOG_INFO("Scene UBO size: ", sizeof(SceneUniforms), " bytes");
    PYRAMID_LOG_INFO("Material UBO size: ", sizeof(MaterialUniforms), " bytes");

    // Bind uniform buffers to shader binding points (this is the missing piece!)
    if (m_shader)
    {
        m_shader->Bind();
        m_shader->BindUniformBuffer("SceneData", m_sceneUBO.get(), 0);
        m_shader->BindUniformBuffer("MaterialData", m_materialUBO.get(), 1);
        m_shader->Unbind();
        PYRAMID_LOG_INFO("Bound uniform buffers to shader binding points");
    }
}

void BasicGame::UpdateUniformBuffers(float deltaTime)
{
    if (!m_sceneUBO || !m_materialUBO)
        return;

    // Update scene data with animated values
    SceneUniforms sceneData = {};

    // Create animated camera position
    float radius = 2.0f;
    float cameraX = radius * cosf(m_time * 0.3f);
    float cameraZ = radius * sinf(m_time * 0.3f);

    // Simple look-at matrix (simplified for demo)
    memset(sceneData.viewMatrix, 0, sizeof(sceneData.viewMatrix));
    sceneData.viewMatrix[0] = 1.0f;
    sceneData.viewMatrix[5] = 1.0f;
    sceneData.viewMatrix[10] = 1.0f;
    sceneData.viewMatrix[15] = 1.0f;
    sceneData.viewMatrix[14] = -3.0f; // Move camera back

    // Keep the same projection matrix
    memset(sceneData.projectionMatrix, 0, sizeof(sceneData.projectionMatrix));
    float fov = 45.0f * 3.14159f / 180.0f;
    float aspect = 800.0f / 600.0f;
    float near = 0.1f, far = 100.0f;
    float f = 1.0f / tanf(fov * 0.5f);
    sceneData.projectionMatrix[0] = f / aspect;
    sceneData.projectionMatrix[5] = f;
    sceneData.projectionMatrix[10] = (far + near) / (near - far);
    sceneData.projectionMatrix[11] = -1.0f;
    sceneData.projectionMatrix[14] = (2.0f * far * near) / (near - far);

    // Update camera position
    sceneData.cameraPosition[0] = cameraX;
    sceneData.cameraPosition[1] = 0.5f;
    sceneData.cameraPosition[2] = cameraZ;
    sceneData.cameraPosition[3] = 1.0f;

    // Animate light direction
    sceneData.lightDirection[0] = sinf(m_time * 0.5f);
    sceneData.lightDirection[1] = -0.8f;
    sceneData.lightDirection[2] = cosf(m_time * 0.5f);
    sceneData.lightDirection[3] = 0.0f;

    sceneData.time = m_time;

    // Update material data with animated values
    MaterialUniforms materialData = {};
    materialData.baseColor[0] = 0.8f + 0.2f * sinf(m_time * 0.7f);
    materialData.baseColor[1] = 0.8f + 0.2f * sinf(m_time * 0.9f + 2.0f);
    materialData.baseColor[2] = 0.8f + 0.2f * sinf(m_time * 1.1f + 4.0f);
    materialData.baseColor[3] = 1.0f;

    materialData.emissiveColor[0] = 0.1f + 0.1f * sinf(m_time * 2.0f);
    materialData.emissiveColor[1] = 0.2f + 0.2f * sinf(m_time * 1.5f + 1.0f);
    materialData.emissiveColor[2] = 0.4f + 0.4f * sinf(m_time * 1.8f + 3.0f);
    materialData.emissiveColor[3] = 1.0f;

    materialData.metallic = 0.5f + 0.5f * sinf(m_time * 0.4f);
    materialData.roughness = 0.3f + 0.4f * sinf(m_time * 0.6f + 1.5f);
    materialData.textureScale = 1.0f + 0.5f * sinf(m_time * 0.8f);

    // Upload updated data to uniform buffers
    m_sceneUBO->UpdateData(&sceneData, sizeof(SceneUniforms));
    m_materialUBO->UpdateData(&materialData, sizeof(MaterialUniforms));
}

void BasicGame::onUpdate(float deltaTime)
{
    Game::onUpdate(deltaTime);
    m_time += deltaTime;
    m_textureSwapTimer += deltaTime;

    // Update uniform buffers with animated data
    UpdateUniformBuffers(deltaTime);

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
    if (m_shader && m_vertexArray && m_sceneUBO && m_materialUBO)
    {
        m_shader->Bind();

        // Bind uniform buffers to their respective binding points
        // Note: In a real implementation, we'd use glBindBufferBase or similar
        // For now, we'll rely on the shader's uniform block bindings

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
            // Create a simple white texture as fallback
            static bool fallbackCreated = false;
            if (!fallbackCreated)
            {
                PYRAMID_LOG_INFO("No textures available, using uniform buffer animation only");
                fallbackCreated = true;
            }
        }

        m_vertexArray->Bind();
        GetGraphicsDevice()->DrawIndexed(m_vertexArray->GetIndexBuffer()->GetCount());
        m_vertexArray->Unbind();
        m_shader->Unbind();
    }

    Game::onRender();
}
