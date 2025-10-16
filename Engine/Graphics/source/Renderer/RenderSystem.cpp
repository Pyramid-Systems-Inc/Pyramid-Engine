#include <Pyramid/Graphics/Renderer/RenderSystem.hpp>
#include <Pyramid/Graphics/Renderer/RenderPasses.hpp>
#include <Pyramid/Graphics/GraphicsDevice.hpp>
#include <Pyramid/Graphics/Camera.hpp>
#include <Pyramid/Graphics/Shader/Shader.hpp>
#include <Pyramid/Graphics/Texture.hpp>
#include <Pyramid/Graphics/Buffer/UniformBuffer.hpp>
#include <Pyramid/Util/Log.hpp>
#include <glad/glad.h>
#include <algorithm>
#include <chrono>

namespace Pyramid
{
    namespace Renderer
    {

        // CommandBuffer Implementation
        CommandBuffer::CommandBuffer()
        {
            m_commands.reserve(1024); // Pre-allocate for performance
        }

        CommandBuffer::~CommandBuffer() = default;

        void CommandBuffer::Begin()
        {
            m_recording = true;
            m_commands.clear();
        }

        void CommandBuffer::End()
        {
            m_recording = false;
        }

        void CommandBuffer::Reset()
        {
            m_commands.clear();
            m_recording = false;
        }

        void CommandBuffer::SetRenderTarget(u32 targetId)
        {
            if (!m_recording) return;
            
            RenderCommand cmd;
            cmd.type = RenderCommandType::SetRenderTarget;
            cmd.data.setRenderTarget.targetId = targetId;
            m_commands.push_back(cmd);
        }

        void CommandBuffer::SetShader(u32 shaderId)
        {
            if (!m_recording) return;
            
            RenderCommand cmd;
            cmd.type = RenderCommandType::SetShader;
            cmd.data.setShader.shaderId = shaderId;
            m_commands.push_back(cmd);
        }

        void CommandBuffer::SetTexture(u32 textureId, u32 slot)
        {
            if (!m_recording) return;
            
            RenderCommand cmd;
            cmd.type = RenderCommandType::SetTexture;
            cmd.data.setTexture.textureId = textureId;
            cmd.data.setTexture.slot = slot;
            m_commands.push_back(cmd);
        }

        void CommandBuffer::SetUniformBuffer(u32 bufferId, u32 bindingPoint)
        {
            if (!m_recording) return;
            
            RenderCommand cmd;
            cmd.type = RenderCommandType::SetUniformBuffer;
            cmd.data.setUniformBuffer.bufferId = bufferId;
            cmd.data.setUniformBuffer.bindingPoint = bindingPoint;
            m_commands.push_back(cmd);
        }

        void CommandBuffer::DrawIndexed(u32 indexCount, u32 instanceCount)
        {
            if (!m_recording) return;
            
            RenderCommand cmd;
            cmd.type = RenderCommandType::DrawIndexed;
            cmd.data.draw.indexCount = indexCount;
            cmd.data.draw.instanceCount = instanceCount;
            m_commands.push_back(cmd);
        }

        void CommandBuffer::Dispatch(u32 x, u32 y, u32 z)
        {
            if (!m_recording) return;
            
            RenderCommand cmd;
            cmd.type = RenderCommandType::Dispatch;
            cmd.data.dispatch.x = x;
            cmd.data.dispatch.y = y;
            cmd.data.dispatch.z = z;
            m_commands.push_back(cmd);
        }

        void CommandBuffer::ClearTarget(f32 r, f32 g, f32 b, f32 a)
        {
            if (!m_recording) return;
            
            RenderCommand cmd;
            cmd.type = RenderCommandType::ClearTarget;
            cmd.data.clear.r = r;
            cmd.data.clear.g = g;
            cmd.data.clear.b = b;
            cmd.data.clear.a = a;
            m_commands.push_back(cmd);
        }

        void CommandBuffer::Execute(IGraphicsDevice* device)
        {
            if (!device) {
                PYRAMID_LOG_ERROR("Cannot execute command buffer: device is null");
                return;
            }

            for (const auto& cmd : m_commands) {
                switch (cmd.type) {
                    case RenderCommandType::SetRenderTarget:
                        // Note: RenderTarget binding will be handled through resource lookup
                        // For now, log that the command was received
                        PYRAMID_LOG_DEBUG("SetRenderTarget command: targetId=", cmd.data.setRenderTarget.targetId);
                        break;
                        
                    case RenderCommandType::SetShader:
                        // Note: Shader binding will be handled through resource lookup
                        PYRAMID_LOG_DEBUG("SetShader command: shaderId=", cmd.data.setShader.shaderId);
                        break;
                        
                    case RenderCommandType::SetTexture:
                        // Note: Texture binding will be handled through resource lookup
                        PYRAMID_LOG_DEBUG("SetTexture command: textureId=", cmd.data.setTexture.textureId, " slot=", cmd.data.setTexture.slot);
                        break;
                        
                    case RenderCommandType::SetUniformBuffer:
                        // Note: UBO binding will be handled through resource lookup
                        PYRAMID_LOG_DEBUG("SetUniformBuffer command: bufferId=", cmd.data.setUniformBuffer.bufferId, " bindingPoint=", cmd.data.setUniformBuffer.bindingPoint);
                        break;
                        
                    case RenderCommandType::ClearTarget:
                        device->Clear(Color(cmd.data.clear.r, cmd.data.clear.g,
                                          cmd.data.clear.b, cmd.data.clear.a));
                        break;
                    
                    case RenderCommandType::DrawIndexed:
                        if (cmd.data.draw.instanceCount > 1) {
                            device->DrawIndexedInstanced(cmd.data.draw.indexCount, cmd.data.draw.instanceCount);
                        } else {
                            device->DrawIndexed(cmd.data.draw.indexCount);
                        }
                        break;
                        
                    case RenderCommandType::DrawInstanced:
                        device->DrawIndexedInstanced(cmd.data.draw.indexCount, cmd.data.draw.instanceCount);
                        break;
                    
                    case RenderCommandType::Dispatch:
                        PYRAMID_LOG_DEBUG("Compute dispatch command: ", cmd.data.dispatch.x, "x", cmd.data.dispatch.y, "x", cmd.data.dispatch.z);
                        // Dispatch will be handled when compute shader support is added
                        break;
                    
                    default:
                        PYRAMID_LOG_WARN("Unknown render command type: ", static_cast<int>(cmd.type));
                        break;
                }
            }
        }

        // RenderTarget Implementation
        RenderTarget::RenderTarget(const RenderTargetSpec& spec)
            : m_spec(spec)
        {
        }

        RenderTarget::~RenderTarget()
        {
            if (m_framebuffer != 0)
            {
                glDeleteFramebuffers(1, &m_framebuffer);
            }
            
            if (!m_colorTextures.empty())
            {
                glDeleteTextures(static_cast<GLsizei>(m_colorTextures.size()), m_colorTextures.data());
            }
            
            if (m_depthTexture != 0)
            {
                glDeleteTextures(1, &m_depthTexture);
            }
        }

        bool RenderTarget::Initialize(IGraphicsDevice* device)
        {
            if (!device)
            {
                PYRAMID_LOG_ERROR("Cannot initialize render target: device is null");
                return false;
            }

            // Generate framebuffer
            glGenFramebuffers(1, &m_framebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

            // Create color attachments
            m_colorTextures.resize(m_spec.colorAttachments);
            glGenTextures(m_spec.colorAttachments, m_colorTextures.data());

            for (u32 i = 0; i < m_spec.colorAttachments; i++)
            {
                glBindTexture(GL_TEXTURE_2D, m_colorTextures[i]);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_spec.width, m_spec.height, 0, GL_RGBA, GL_FLOAT, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_colorTextures[i], 0);
            }

            // Create depth-stencil attachment if needed
            if (m_spec.hasDepthStencil)
            {
                glGenTextures(1, &m_depthTexture);
                glBindTexture(GL_TEXTURE_2D, m_depthTexture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_spec.width, m_spec.height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);
            }

            // Set draw buffers
            if (m_spec.colorAttachments > 0)
            {
                std::vector<GLenum> drawBuffers;
                for (u32 i = 0; i < m_spec.colorAttachments; i++)
                {
                    drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
                }
                glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
            }

            // Check framebuffer completeness
            GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (status != GL_FRAMEBUFFER_COMPLETE)
            {
                PYRAMID_LOG_ERROR("Render target framebuffer not complete: ", status);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                return false;
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            m_initialized = true;
            
            PYRAMID_LOG_INFO("Render target initialized: ", m_spec.width, "x", m_spec.height,
                           " with ", m_spec.colorAttachments, " color attachments");
            return true;
        }

        void RenderTarget::Bind()
        {
            glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
            glViewport(0, 0, m_spec.width, m_spec.height);
        }

        void RenderTarget::Unbind()
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        void RenderTarget::Clear(f32 r, f32 g, f32 b, f32 a)
        {
            Bind();
            glClearColor(r, g, b, a);
            GLbitfield clearMask = GL_COLOR_BUFFER_BIT;
            if (m_spec.hasDepthStencil)
            {
                clearMask |= GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
            }
            glClear(clearMask);
        }

        u32 RenderTarget::GetColorTexture(u32 index) const
        {
            if (index < m_colorTextures.size())
            {
                return m_colorTextures[index];
            }
            return 0;
        }

        u32 RenderTarget::GetDepthTexture() const
        {
            return m_depthTexture;
        }

        // RenderPass Implementation
        RenderPass::RenderPass(RenderPassType type, const std::string& name)
            : m_type(type), m_name(name)
        {
        }

        // RenderSystem Implementation
        RenderSystem::RenderSystem()
        {
            m_mainCommandBuffer = std::make_unique<CommandBuffer>();
            m_shadowCommandBuffer = std::make_unique<CommandBuffer>();
        }

        RenderSystem::~RenderSystem()
        {
            Shutdown();
        }

        bool RenderSystem::Initialize(IGraphicsDevice* device)
        {
            if (!device) {
                PYRAMID_LOG_ERROR("Cannot initialize render system: device is null");
                return false;
            }

            m_device = device;

            // Create global uniform buffers
            struct CameraUniforms {
                Math::Mat4 viewMatrix;
                Math::Mat4 projectionMatrix;
                Math::Mat4 viewProjectionMatrix;
                Math::Vec4 cameraPosition;
                Math::Vec4 cameraDirection;
                f32 nearPlane;
                f32 farPlane;
                f32 fov;
                f32 aspectRatio;
            };

            m_cameraUBO = device->CreateUniformBuffer(sizeof(CameraUniforms));
            if (!m_cameraUBO) {
                PYRAMID_LOG_ERROR("Failed to create camera uniform buffer");
                return false;
            }

            m_lightingUBO = device->CreateUniformBuffer(sizeof(LightingData));
            if (!m_lightingUBO) {
                PYRAMID_LOG_ERROR("Failed to create lighting uniform buffer");
                return false;
            }

            // Setup default render passes
            SetupDefaultRenderPasses();

            PYRAMID_LOG_INFO("Render system initialized successfully");
            PYRAMID_LOG_INFO("  Camera UBO size: ", sizeof(CameraUniforms), " bytes");
            PYRAMID_LOG_INFO("  Lighting UBO size: ", sizeof(LightingData), " bytes");
            PYRAMID_LOG_INFO("  Default render passes: ", m_renderPasses.size());

            return true;
        }

        void RenderSystem::Shutdown()
        {
            m_renderPasses.clear();
            m_renderTargets.clear();
            m_cameraUBO.reset();
            m_lightingUBO.reset();
            m_device = nullptr;

            PYRAMID_LOG_INFO("Render system shut down");
        }

        void RenderSystem::BeginFrame()
        {
            if (!m_device) return;

            // Reset statistics
            m_stats = {};
            
            // Begin main command buffer
            m_mainCommandBuffer->Begin();
        }

        void RenderSystem::Render(const Scene& scene, const Camera& camera)
        {
            if (!m_device) return;

            auto frameStart = std::chrono::high_resolution_clock::now();

            // Update global uniforms
            UpdateGlobalUniforms(camera);

            // Execute render passes in order
            for (auto& pass : m_renderPasses) {
                if (!pass->IsEnabled()) continue;

                pass->Begin(*m_mainCommandBuffer);
                pass->Execute(*m_mainCommandBuffer, scene, camera);
                pass->End(*m_mainCommandBuffer);
            }

            // Calculate frame time
            auto frameEnd = std::chrono::high_resolution_clock::now();
            m_stats.frameTime = std::chrono::duration<f32, std::milli>(frameEnd - frameStart).count();
        }

        void RenderSystem::EndFrame()
        {
            if (!m_device) return;

            // End command buffer recording
            m_mainCommandBuffer->End();

            // Execute all commands
            m_mainCommandBuffer->Execute(m_device);

            // Present frame
            m_device->Present(m_vsyncEnabled);

            // Reset command buffer for next frame
            m_mainCommandBuffer->Reset();
        }

        void RenderSystem::AddRenderPass(std::shared_ptr<RenderPass> pass)
        {
            if (!pass) return;

            // Insert pass in correct order based on type
            auto insertPos = std::lower_bound(m_renderPasses.begin(), m_renderPasses.end(), pass,
                [](const std::shared_ptr<RenderPass>& a, const std::shared_ptr<RenderPass>& b) {
                    return static_cast<int>(a->GetType()) < static_cast<int>(b->GetType());
                });

            m_renderPasses.insert(insertPos, pass);
            PYRAMID_LOG_INFO("Added render pass: ", pass->GetName());
        }

        void RenderSystem::RemoveRenderPass(RenderPassType type)
        {
            auto it = std::find_if(m_renderPasses.begin(), m_renderPasses.end(),
                [type](const std::shared_ptr<RenderPass>& pass) {
                    return pass->GetType() == type;
                });

            if (it != m_renderPasses.end()) {
                PYRAMID_LOG_INFO("Removed render pass: ", (*it)->GetName());
                m_renderPasses.erase(it);
            }
        }

        std::shared_ptr<RenderPass> RenderSystem::GetRenderPass(RenderPassType type)
        {
            auto it = std::find_if(m_renderPasses.begin(), m_renderPasses.end(),
                [type](const std::shared_ptr<RenderPass>& pass) {
                    return pass->GetType() == type;
                });

            return (it != m_renderPasses.end()) ? *it : nullptr;
        }

        void RenderSystem::SetupDefaultRenderPasses()
        {
            // Create shadow map pass (renders first)
            auto shadowPass = std::make_shared<ShadowMapPass>("Shadow", m_device, 4);
            shadowPass->SetShadowMapResolution(2048);
            shadowPass->SetDepthBias(0.005f);
            AddRenderPass(shadowPass);
            
            // Create forward render pass
            auto forwardPass = std::make_shared<ForwardRenderPass>();
            forwardPass->SetClearColor(Math::Vec4(0.1f, 0.1f, 0.15f, 1.0f));
            AddRenderPass(forwardPass);
            
            // Future: Add post-process, UI passes
            
            PYRAMID_LOG_INFO("Default render passes initialized with shadows");
        }

        void RenderSystem::SetupDeferredPipeline()
        {
            // Clear existing render passes
            m_renderPasses.clear();
            
            // Get viewport dimensions (assuming 1920x1080 default, should be updated dynamically)
            u32 width = 1920;
            u32 height = 1080;
            
            // Shadow pass
            auto shadowPass = std::make_shared<ShadowMapPass>("Shadow", m_device, 4);
            shadowPass->SetShadowMapResolution(2048);
            shadowPass->SetDepthBias(0.005f);
            AddRenderPass(shadowPass);
            
            // Deferred geometry pass
            auto geometryPass = std::make_shared<DeferredGeometryPass>("DeferredGeometry", m_device, width, height);
            AddRenderPass(geometryPass);
            
            // Deferred lighting pass
            auto lightingPass = std::make_shared<DeferredLightingPass>("DeferredLighting", m_device);
            
            // Connect G-Buffer from geometry pass to lighting pass
            lightingPass->SetGBuffer(geometryPass->GetGBuffer());
            
            // Connect shadow maps from shadow pass to lighting pass
            lightingPass->SetShadowMaps(shadowPass->GetShadowMaps());
            
            AddRenderPass(lightingPass);
            
            PYRAMID_LOG_INFO("Deferred rendering pipeline initialized");
        }

        void RenderSystem::UpdateGlobalUniforms(const Camera& camera)
        {
            // Update camera UBO
            if (m_cameraUBO) {
                struct CameraUniforms {
                    Math::Mat4 viewMatrix;
                    Math::Mat4 projectionMatrix;
                    Math::Mat4 viewProjectionMatrix;
                    Math::Vec4 cameraPosition;
                    Math::Vec4 cameraDirection;
                    f32 nearPlane;
                    f32 farPlane;
                    f32 fov;
                    f32 aspectRatio;
                };
                
                CameraUniforms cameraData;
                cameraData.viewMatrix = camera.GetViewMatrix();
                cameraData.projectionMatrix = camera.GetProjectionMatrix();
                cameraData.viewProjectionMatrix = camera.GetViewProjectionMatrix();
                cameraData.cameraPosition = Math::Vec4(camera.GetPosition(), 1.0f);
                cameraData.cameraDirection = Math::Vec4(camera.GetForward(), 0.0f);
                cameraData.nearPlane = camera.GetNearPlane();
                cameraData.farPlane = camera.GetFarPlane();
                cameraData.fov = camera.GetFOV();
                cameraData.aspectRatio = camera.GetAspectRatio();
                
                m_cameraUBO->UpdateData(&cameraData, sizeof(CameraUniforms), 0);
                m_cameraUBO->Bind(0); // Bind to binding point 0
            }
            
            // Update lighting UBO
            if (m_lightingUBO) {
                LightingData lightingData;
                // Use default lighting for now - can be extended to use scene lights
                m_lightingUBO->UpdateData(&lightingData, sizeof(LightingData), 0);
                m_lightingUBO->Bind(1); // Bind to binding point 1
            }
        }

        void RenderSystem::BindMaterial(CommandBuffer& cmdBuffer, const Material& material)
        {
            // Bind shader
            if (material.shader) {
                material.shader->Bind();
            }
            
            // Bind textures to their respective slots
            if (material.albedoTexture) {
                material.albedoTexture->Bind(0); // Albedo at slot 0
            }
            
            if (material.normalTexture) {
                material.normalTexture->Bind(1); // Normal at slot 1
            }
            
            if (material.metallicRoughnessTexture) {
                material.metallicRoughnessTexture->Bind(2); // MetallicRoughness at slot 2
            }
            
            if (material.aoTexture) {
                material.aoTexture->Bind(3); // AO at slot 3
            }
            
            if (material.emissiveTexture) {
                material.emissiveTexture->Bind(4); // Emissive at slot 4
            }
            
            // Material properties can be set as shader uniforms if needed
            // This will be handled by the render pass when setting per-material uniforms
        }

    } // namespace Renderer
} // namespace Pyramid
