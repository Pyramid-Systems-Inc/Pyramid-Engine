#include <Pyramid/Graphics/Renderer/RenderPasses.hpp>
#include <Pyramid/Graphics/GraphicsDevice.hpp>
#include <Pyramid/Graphics/Scene.hpp>
#include <Pyramid/Graphics/Camera.hpp>
#include <Pyramid/Graphics/Shader/Shader.hpp>
#include <Pyramid/Graphics/Texture.hpp>
#include <Pyramid/Graphics/OpenGL/OpenGLFramebuffer.hpp>
#include <Pyramid/Graphics/Buffer/VertexArray.hpp>
#include <Pyramid/Graphics/Buffer/VertexBuffer.hpp>
#include <Pyramid/Graphics/Buffer/BufferLayout.hpp>
#include <Pyramid/Util/Log.hpp>
#include <glad/glad.h>
#include <fstream>
#include <sstream>

namespace Pyramid
{
    namespace Renderer
    {

        DeferredLightingPass::DeferredLightingPass(const std::string& name, IGraphicsDevice* device)
            : RenderPass(RenderPassType::Lighting, name)
            , m_device(device)
        {
            PYRAMID_LOG_INFO("DeferredLightingPass created");
            
            // Create lighting shader
            m_lightingShader = m_device->CreateShader();
            
            // Load shader source from files
            std::ifstream vertFile("Engine/Graphics/shaders/deferred_lighting.vert");
            std::ifstream fragFile("Engine/Graphics/shaders/deferred_lighting.frag");
            
            if (!vertFile.is_open() || !fragFile.is_open())
            {
                PYRAMID_LOG_ERROR("Failed to open deferred lighting shader files");
            }
            else
            {
                std::stringstream vertStream, fragStream;
                vertStream << vertFile.rdbuf();
                fragStream << fragFile.rdbuf();
                
                std::string vertSrc = vertStream.str();
                std::string fragSrc = fragStream.str();
                
                if (!m_lightingShader->Compile(vertSrc, fragSrc))
                {
                    PYRAMID_LOG_ERROR("Failed to compile deferred lighting shaders");
                }
                else
                {
                    PYRAMID_LOG_INFO("Deferred lighting shaders compiled successfully");
                }
            }
            
            // Create fullscreen quad
            CreateFullscreenQuad();
        }

        void DeferredLightingPass::CreateFullscreenQuad()
        {
            // Fullscreen triangle (more efficient than quad - covers entire screen with one triangle)
            float vertices[] = {
                // Positions      // TexCoords
                -1.0f, -1.0f,     0.0f, 0.0f,
                 3.0f, -1.0f,     2.0f, 0.0f,
                -1.0f,  3.0f,     0.0f, 2.0f
            };
            
            auto vbo = m_device->CreateVertexBuffer();
            vbo->SetData(vertices, sizeof(vertices));
            
            BufferLayout layout = {
                { ShaderDataType::Float2, "a_Position" },
                { ShaderDataType::Float2, "a_TexCoord" }
            };
            
            m_fullscreenQuad = m_device->CreateVertexArray();
            m_fullscreenQuad->AddVertexBuffer(vbo, layout);
            
            PYRAMID_LOG_DEBUG("Fullscreen quad created for deferred lighting");
        }

        void DeferredLightingPass::Begin(CommandBuffer& cmd)
        {
            // Bind default framebuffer (render to screen)
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            
            // Clear screen
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            // Disable depth testing (fullscreen quad doesn't need it)
            glDisable(GL_DEPTH_TEST);
            
            // Disable face culling
            glDisable(GL_CULL_FACE);
            
            PYRAMID_LOG_DEBUG("DeferredLightingPass::Begin");
        }

        void DeferredLightingPass::Execute(CommandBuffer& cmd, const Scene& scene, const Camera& camera)
        {
            if (!m_lightingShader || !m_fullscreenQuad || !m_gBuffer)
            {
                PYRAMID_LOG_WARN("DeferredLightingPass not properly initialized");
                return;
            }
            
            // Bind lighting shader
            m_lightingShader->Bind();
            
            // Bind G-Buffer textures
            GLuint albedoMetallic = m_gBuffer->GetColorAttachmentTexture(0);
            GLuint normalRoughness = m_gBuffer->GetColorAttachmentTexture(1);
            GLuint positionAO = m_gBuffer->GetColorAttachmentTexture(2);
            GLuint emissive = m_gBuffer->GetColorAttachmentTexture(3);
            GLuint depth = m_gBuffer->GetDepthAttachmentTexture();
            
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, albedoMetallic);
            m_lightingShader->SetUniformInt("u_GAlbedoMetallic", 0);
            
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, normalRoughness);
            m_lightingShader->SetUniformInt("u_GNormalRoughness", 1);
            
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, positionAO);
            m_lightingShader->SetUniformInt("u_GPositionAO", 2);
            
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, emissive);
            m_lightingShader->SetUniformInt("u_GEmissive", 3);
            
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, depth);
            m_lightingShader->SetUniformInt("u_GDepth", 4);
            
            // Bind shadow maps if available
            if (!m_shadowMaps.empty())
            {
                // For now, bind first shadow map cascade
                // TODO: Implement shadow map array binding
                GLuint shadowMap = m_shadowMaps[0]->GetDepthAttachmentTexture();
                glActiveTexture(GL_TEXTURE5);
                glBindTexture(GL_TEXTURE_2D, shadowMap);
                m_lightingShader->SetUniformInt("u_ShadowMaps", 5);
            }
            
            // Set camera uniforms
            Math::Vec3 camPos = camera.GetPosition();
            m_lightingShader->SetUniformFloat3("u_CameraPosition", camPos.x, camPos.y, camPos.z);
            
            // Get primary directional light from scene
            auto primaryLight = scene.GetPrimaryLight();
            if (primaryLight && primaryLight->enabled)
            {
                Math::Vec3 lightDir = primaryLight->direction.Normalized();
                m_lightingShader->SetUniformFloat3("u_LightDirection", lightDir.x, lightDir.y, lightDir.z);
                m_lightingShader->SetUniformFloat3("u_LightColor", 
                    primaryLight->color.x, 
                    primaryLight->color.y, 
                    primaryLight->color.z);
                m_lightingShader->SetUniformFloat("u_LightIntensity", primaryLight->intensity);
            }
            else
            {
                // Default lighting
                m_lightingShader->SetUniformFloat3("u_LightDirection", 0.5f, -1.0f, 0.5f);
                m_lightingShader->SetUniformFloat3("u_LightColor", 1.0f, 1.0f, 1.0f);
                m_lightingShader->SetUniformFloat("u_LightIntensity", 1.0f);
            }
            
            // Set shadow parameters
            m_lightingShader->SetUniformFloat("u_ShadowBias", 0.005f);
            m_lightingShader->SetUniformInt("u_CascadeCount", static_cast<int>(m_shadowMaps.size()));
            
            // Set technique flags
            m_lightingShader->SetUniformInt("u_EnableSSAO", m_enableSSAO ? 1 : 0);
            m_lightingShader->SetUniformInt("u_EnableIBL", m_enableIBL ? 1 : 0);
            
            // Draw fullscreen quad
            m_fullscreenQuad->Bind();
            glDrawArrays(GL_TRIANGLES, 0, 3);
            
            PYRAMID_LOG_DEBUG("DeferredLightingPass::Execute - Fullscreen lighting applied");
        }

        void DeferredLightingPass::End(CommandBuffer& cmd)
        {
            // Re-enable depth testing for subsequent passes
            glEnable(GL_DEPTH_TEST);
            
            // Unbind textures
            for (int i = 0; i < 6; i++)
            {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            
            PYRAMID_LOG_DEBUG("DeferredLightingPass::End");
        }

        void DeferredLightingPass::SetGBuffer(std::shared_ptr<OpenGLFramebuffer> gBuffer)
        {
            m_gBuffer = gBuffer;
            PYRAMID_LOG_DEBUG("G-Buffer set for deferred lighting pass");
        }

        void DeferredLightingPass::SetShadowMaps(const std::vector<std::shared_ptr<OpenGLFramebuffer>>& shadowMaps)
        {
            m_shadowMaps = shadowMaps;
            PYRAMID_LOG_DEBUG("Shadow maps set for deferred lighting pass (", shadowMaps.size(), " cascades)");
        }

    } // namespace Renderer
} // namespace Pyramid