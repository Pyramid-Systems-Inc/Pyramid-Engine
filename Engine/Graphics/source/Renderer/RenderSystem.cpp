#include <Pyramid/Graphics/Renderer/RenderSystem.hpp>
#include <Pyramid/Graphics/GraphicsDevice.hpp>
#include <Pyramid/Graphics/Buffer/UniformBuffer.hpp>
#include <Pyramid/Util/Log.hpp>
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
                        // TODO: Implement render target binding when framebuffer system is complete
                        PYRAMID_LOG_DEBUG("SetRenderTarget command executed (placeholder)");
                        break;
                        
                    case RenderCommandType::SetShader:
                        // TODO: Implement shader binding when shader system is enhanced
                        PYRAMID_LOG_DEBUG("SetShader command executed (placeholder)");
                        break;
                        
                    case RenderCommandType::SetTexture:
                        // TODO: Implement texture binding when texture system is enhanced
                        PYRAMID_LOG_DEBUG("SetTexture command executed (placeholder)");
                        break;
                        
                    case RenderCommandType::SetUniformBuffer:
                        // TODO: Implement UBO binding when uniform buffer system is enhanced
                        PYRAMID_LOG_DEBUG("SetUniformBuffer command executed (placeholder)");
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
                        // TODO: Implement compute shader dispatch when compute shaders are supported
                        PYRAMID_LOG_DEBUG("Compute dispatch command executed (placeholder)");
                        break;
                    
                    default:
                        PYRAMID_LOG_WARN("Unknown render command type: ", static_cast<int>(cmd.type));
                        break;
                }
            }
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
            // TODO: Create default render passes (forward rendering for now)
            // This will be expanded when we implement specific render passes
            PYRAMID_LOG_INFO("Default render passes setup (placeholder)");
        }

        void RenderSystem::UpdateGlobalUniforms(const Camera& camera)
        {
            // TODO: Update camera and lighting uniforms
            // This requires Camera class implementation
            PYRAMID_LOG_DEBUG("Updated global uniforms (placeholder)");
        }

    } // namespace Renderer
} // namespace Pyramid
