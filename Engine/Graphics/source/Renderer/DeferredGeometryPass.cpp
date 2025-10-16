#include <Pyramid/Graphics/Renderer/RenderPasses.hpp>
#include <Pyramid/Graphics/GraphicsDevice.hpp>
#include <Pyramid/Graphics/Scene.hpp>
#include <Pyramid/Graphics/Camera.hpp>
#include <Pyramid/Graphics/Shader/Shader.hpp>
#include <Pyramid/Graphics/Texture.hpp>
#include <Pyramid/Graphics/OpenGL/OpenGLFramebuffer.hpp>
#include <Pyramid/Graphics/Buffer/VertexArray.hpp>
#include <Pyramid/Graphics/Buffer/IndexBuffer.hpp>
#include <Pyramid/Util/Log.hpp>
#include <glad/glad.h>
#include <fstream>
#include <sstream>

namespace Pyramid
{
    namespace Renderer
    {

        DeferredGeometryPass::DeferredGeometryPass(const std::string& name, IGraphicsDevice* device, u32 width, u32 height)
            : RenderPass(RenderPassType::GBuffer, name)
            , m_device(device)
            , m_width(width)
            , m_height(height)
        {
            PYRAMID_LOG_INFO("DeferredGeometryPass created at ", width, "x", height);
            
            // Create geometry shader
            m_geometryShader = m_device->CreateShader();
            
            // Load shader source from files
            std::ifstream vertFile("Engine/Graphics/shaders/deferred_geometry.vert");
            std::ifstream fragFile("Engine/Graphics/shaders/deferred_geometry.frag");
            
            if (!vertFile.is_open() || !fragFile.is_open())
            {
                PYRAMID_LOG_ERROR("Failed to open deferred geometry shader files");
            }
            else
            {
                std::stringstream vertStream, fragStream;
                vertStream << vertFile.rdbuf();
                fragStream << fragFile.rdbuf();
                
                std::string vertSrc = vertStream.str();
                std::string fragSrc = fragStream.str();
                
                if (!m_geometryShader->Compile(vertSrc, fragSrc))
                {
                    PYRAMID_LOG_ERROR("Failed to compile deferred geometry shaders");
                }
                else
                {
                    PYRAMID_LOG_INFO("Deferred geometry shaders compiled successfully");
                }
            }
            
            // Create G-Buffer
            CreateGBuffer();
        }

        void DeferredGeometryPass::CreateGBuffer()
        {
            FramebufferSpec spec;
            spec.width = m_width;
            spec.height = m_height;
            spec.samples = 1; // No MSAA in deferred rendering (can add MSAA resolve later)
            spec.swapChainTarget = false;
            
            // RT0: Albedo (RGB) + Metallic (A) - GL_RGBA8
            FramebufferAttachmentSpec albedoSpec;
            albedoSpec.type = FramebufferAttachmentType::Color;
            albedoSpec.internalFormat = GL_RGBA8;
            albedoSpec.format = GL_RGBA;
            albedoSpec.dataType = GL_UNSIGNED_BYTE;
            albedoSpec.minFilter = GL_NEAREST;
            albedoSpec.magFilter = GL_NEAREST;
            albedoSpec.colorAttachmentIndex = 0;
            spec.attachments.push_back(albedoSpec);
            
            // RT1: Normal (RGB) + Roughness (A) - GL_RGBA16F
            FramebufferAttachmentSpec normalSpec;
            normalSpec.type = FramebufferAttachmentType::Color;
            normalSpec.internalFormat = GL_RGBA16F;
            normalSpec.format = GL_RGBA;
            normalSpec.dataType = GL_HALF_FLOAT;
            normalSpec.minFilter = GL_NEAREST;
            normalSpec.magFilter = GL_NEAREST;
            normalSpec.colorAttachmentIndex = 1;
            spec.attachments.push_back(normalSpec);
            
            // RT2: WorldPos (RGB) + AO (A) - GL_RGBA16F
            FramebufferAttachmentSpec positionSpec;
            positionSpec.type = FramebufferAttachmentType::Color;
            positionSpec.internalFormat = GL_RGBA16F;
            positionSpec.format = GL_RGBA;
            positionSpec.dataType = GL_HALF_FLOAT;
            positionSpec.minFilter = GL_NEAREST;
            positionSpec.magFilter = GL_NEAREST;
            positionSpec.colorAttachmentIndex = 2;
            spec.attachments.push_back(positionSpec);
            
            // RT3: Emissive (RGB) + Flags (A) - GL_RGBA16F
            FramebufferAttachmentSpec emissiveSpec;
            emissiveSpec.type = FramebufferAttachmentType::Color;
            emissiveSpec.internalFormat = GL_RGBA16F;
            emissiveSpec.format = GL_RGBA;
            emissiveSpec.dataType = GL_HALF_FLOAT;
            emissiveSpec.minFilter = GL_NEAREST;
            emissiveSpec.magFilter = GL_NEAREST;
            emissiveSpec.colorAttachmentIndex = 3;
            spec.attachments.push_back(emissiveSpec);
            
            // Depth attachment - Depth24Stencil8
            FramebufferAttachmentSpec depthSpec;
            depthSpec.type = FramebufferAttachmentType::DepthStencil;
            depthSpec.internalFormat = GL_DEPTH24_STENCIL8;
            depthSpec.format = GL_DEPTH_STENCIL;
            depthSpec.dataType = GL_UNSIGNED_INT_24_8;
            spec.attachments.push_back(depthSpec);
            
            m_gBuffer = std::make_shared<OpenGLFramebuffer>(spec);
            if (!m_gBuffer->Initialize())
            {
                PYRAMID_LOG_ERROR("Failed to initialize G-Buffer");
            }
            else
            {
                m_gBuffer->SetDebugLabel("G-Buffer");
                PYRAMID_LOG_INFO("G-Buffer created successfully");
            }
        }

        void DeferredGeometryPass::Begin(CommandBuffer& cmd)
        {
            // Bind G-Buffer
            if (m_gBuffer)
            {
                m_gBuffer->Bind();
                m_gBuffer->Clear(0.0f, 0.0f, 0.0f, 0.0f);
            }
            
            // Enable depth testing
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            glDepthMask(GL_TRUE);
            
            // Enable back-face culling
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            
            PYRAMID_LOG_DEBUG("DeferredGeometryPass::Begin - G-Buffer bound");
        }

        void DeferredGeometryPass::Execute(CommandBuffer& cmd, const Scene& scene, const Camera& camera)
        {
            // Get visible objects from the scene
            auto visibleObjects = scene.GetVisibleObjects(camera);
            
            PYRAMID_LOG_DEBUG("DeferredGeometryPass::Execute - Rendering ", visibleObjects.size(), " objects to G-Buffer");
            
            // Render each visible object to G-Buffer
            for (const auto& object : visibleObjects)
            {
                if (!object || !object->visible) continue;
                
                // Skip objects without geometry
                if (!object->vertexArray) {
                    PYRAMID_LOG_WARN("Object '", object->name, "' has no vertex array, skipping");
                    continue;
                }
                
                // Bind geometry shader
                if (m_geometryShader)
                {
                    m_geometryShader->Bind();
                    
                    // Calculate matrices
                    Math::Mat4 model = object->GetTransformMatrix();
                    Math::Mat4 viewProj = camera.GetViewProjectionMatrix();
                    
                    // Set per-object uniforms
                    m_geometryShader->SetUniformMat4("u_Model", model.m);
                    m_geometryShader->SetUniformMat4("u_ViewProjection", viewProj.m);
                    
                    // Calculate normal matrix (inverse transpose of upper-left 3x3)
                    Math::Mat4 normalMatrix = model.Inverse().Transpose();
                    m_geometryShader->SetUniformMat4("u_NormalMatrix", normalMatrix.m);
                    
                    // Set material properties
                    m_geometryShader->SetUniformFloat4("u_AlbedoColor", 
                        object->material.albedo.x, 
                        object->material.albedo.y, 
                        object->material.albedo.z, 
                        object->material.albedo.w);
                    
                    m_geometryShader->SetUniformFloat("u_Metallic", object->material.metallic);
                    m_geometryShader->SetUniformFloat("u_Roughness", object->material.roughness);
                    m_geometryShader->SetUniformFloat3("u_EmissiveColor", 
                        object->material.emissive.x, 
                        object->material.emissive.y, 
                        object->material.emissive.z);
                    m_geometryShader->SetUniformFloat("u_EmissiveIntensity", object->material.emissive.w);
                    
                    // Bind textures if available
                    if (object->material.albedoTexture)
                    {
                        object->material.albedoTexture->Bind(0);
                        m_geometryShader->SetUniformInt("u_AlbedoMap", 0);
                        m_geometryShader->SetUniformInt("u_HasAlbedoMap", 1);
                    }
                    else
                    {
                        m_geometryShader->SetUniformInt("u_HasAlbedoMap", 0);
                    }
                    
                    if (object->material.normalTexture)
                    {
                        object->material.normalTexture->Bind(1);
                        m_geometryShader->SetUniformInt("u_NormalMap", 1);
                        m_geometryShader->SetUniformInt("u_HasNormalMap", 1);
                    }
                    else
                    {
                        m_geometryShader->SetUniformInt("u_HasNormalMap", 0);
                    }
                    
                    if (object->material.metallicRoughnessTexture)
                    {
                        object->material.metallicRoughnessTexture->Bind(2);
                        m_geometryShader->SetUniformInt("u_MetallicRoughnessMap", 2);
                        m_geometryShader->SetUniformInt("u_HasMetallicRoughnessMap", 1);
                    }
                    else
                    {
                        m_geometryShader->SetUniformInt("u_HasMetallicRoughnessMap", 0);
                    }
                    
                    if (object->material.aoTexture)
                    {
                        object->material.aoTexture->Bind(3);
                        m_geometryShader->SetUniformInt("u_AOMap", 3);
                        m_geometryShader->SetUniformInt("u_HasAOMap", 1);
                    }
                    else
                    {
                        m_geometryShader->SetUniformInt("u_HasAOMap", 0);
                    }
                    
                    if (object->material.emissiveTexture)
                    {
                        object->material.emissiveTexture->Bind(4);
                        m_geometryShader->SetUniformInt("u_EmissiveMap", 4);
                        m_geometryShader->SetUniformInt("u_HasEmissiveMap", 1);
                    }
                    else
                    {
                        m_geometryShader->SetUniformInt("u_HasEmissiveMap", 0);
                    }
                }
                else
                {
                    PYRAMID_LOG_WARN("Geometry shader not available");
                    continue;
                }
                
                // Bind vertex array and draw
                object->vertexArray->Bind();
                u32 indexCount = object->vertexArray->GetIndexBuffer()->GetCount();
                cmd.DrawIndexed(indexCount);
            }
        }

        void DeferredGeometryPass::End(CommandBuffer& cmd)
        {
            // Unbind G-Buffer
            if (m_gBuffer)
            {
                m_gBuffer->Unbind();
            }
            
            PYRAMID_LOG_DEBUG("DeferredGeometryPass::End");
        }

        void DeferredGeometryPass::Resize(u32 width, u32 height)
        {
            if (m_width == width && m_height == height)
            {
                return; // No change needed
            }
            
            m_width = width;
            m_height = height;
            
            // Recreate G-Buffer with new dimensions
            CreateGBuffer();
            
            PYRAMID_LOG_INFO("G-Buffer resized to ", width, "x", height);
        }

    } // namespace Renderer
} // namespace Pyramid