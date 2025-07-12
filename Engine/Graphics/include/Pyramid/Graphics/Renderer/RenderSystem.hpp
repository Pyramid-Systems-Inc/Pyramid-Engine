#pragma once

#include <Pyramid/Core/Prerequisites.hpp>
#include <Pyramid/Math/Math.hpp>
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>

namespace Pyramid
{
    // Forward declarations
    class IGraphicsDevice;
    class IShader;
    class ITexture2D;
    class IUniformBuffer;
    class Camera;
    class Scene;

    namespace Renderer
    {

        /**
         * @brief Render pass types for different rendering stages
         */
        enum class RenderPassType
        {
            Shadow,         // Shadow map generation
            GBuffer,        // Geometry buffer for deferred rendering
            Lighting,       // Lighting calculations
            Transparent,    // Transparent objects
            PostProcess,    // Post-processing effects
            UI              // User interface overlay
        };

        /**
         * @brief Render command types for command buffer system
         */
        enum class RenderCommandType
        {
            SetRenderTarget,
            SetShader,
            SetTexture,
            SetUniformBuffer,
            DrawIndexed,
            DrawInstanced,
            Dispatch,       // Compute shader dispatch
            ClearTarget
        };

        /**
         * @brief Individual render command
         */
        struct RenderCommand
        {
            RenderCommandType type;
            union {
                struct { u32 targetId; } setRenderTarget;
                struct { u32 shaderId; } setShader;
                struct { u32 textureId; u32 slot; } setTexture;
                struct { u32 bufferId; u32 bindingPoint; } setUniformBuffer;
                struct { u32 indexCount; u32 instanceCount; } draw;
                struct { u32 x, y, z; } dispatch;
                struct { f32 r, g, b, a; } clear;
            } data;
        };

        /**
         * @brief Command buffer for efficient GPU command submission
         */
        class CommandBuffer
        {
        public:
            CommandBuffer();
            ~CommandBuffer();

            // Command recording
            void Begin();
            void End();
            void Reset();

            // Render commands
            void SetRenderTarget(u32 targetId);
            void SetShader(u32 shaderId);
            void SetTexture(u32 textureId, u32 slot);
            void SetUniformBuffer(u32 bufferId, u32 bindingPoint);
            void DrawIndexed(u32 indexCount, u32 instanceCount = 1);
            void Dispatch(u32 x, u32 y, u32 z);
            void ClearTarget(f32 r, f32 g, f32 b, f32 a);

            // Execution
            void Execute(IGraphicsDevice* device);

            // State
            bool IsRecording() const { return m_recording; }
            size_t GetCommandCount() const { return m_commands.size(); }

        private:
            std::vector<RenderCommand> m_commands;
            bool m_recording = false;
        };

        /**
         * @brief Render target specification
         */
        struct RenderTargetSpec
        {
            u32 width = 1920;
            u32 height = 1080;
            u32 colorAttachments = 1;
            bool hasDepthStencil = true;
            bool multisampled = false;
            u32 samples = 1;
        };

        /**
         * @brief Render target for off-screen rendering
         */
        class RenderTarget
        {
        public:
            RenderTarget(const RenderTargetSpec& spec);
            ~RenderTarget();

            bool Initialize(IGraphicsDevice* device);
            void Bind();
            void Unbind();
            void Clear(f32 r = 0.0f, f32 g = 0.0f, f32 b = 0.0f, f32 a = 1.0f);

            // Accessors
            u32 GetWidth() const { return m_spec.width; }
            u32 GetHeight() const { return m_spec.height; }
            u32 GetColorTexture(u32 index = 0) const;
            u32 GetDepthTexture() const;

        private:
            RenderTargetSpec m_spec;
            u32 m_framebuffer = 0;
            std::vector<u32> m_colorTextures;
            u32 m_depthTexture = 0;
            bool m_initialized = false;
        };

        /**
         * @brief Render pass for organizing rendering stages
         */
        class RenderPass
        {
        public:
            RenderPass(RenderPassType type, const std::string& name);
            virtual ~RenderPass() = default;

            // Execution
            virtual void Begin(CommandBuffer& cmd) = 0;
            virtual void Execute(CommandBuffer& cmd, const Scene& scene, const Camera& camera) = 0;
            virtual void End(CommandBuffer& cmd) = 0;

            // Configuration
            void SetRenderTarget(std::shared_ptr<RenderTarget> target) { m_renderTarget = target; }
            void SetEnabled(bool enabled) { m_enabled = enabled; }

            // Accessors
            RenderPassType GetType() const { return m_type; }
            const std::string& GetName() const { return m_name; }
            bool IsEnabled() const { return m_enabled; }

        protected:
            RenderPassType m_type;
            std::string m_name;
            bool m_enabled = true;
            std::shared_ptr<RenderTarget> m_renderTarget;
        };

        /**
         * @brief Material system for PBR rendering
         */
        struct Material
        {
            Math::Vec4 albedo = Math::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
            Math::Vec4 emissive = Math::Vec4(0.0f, 0.0f, 0.0f, 0.0f);
            f32 metallic = 0.0f;
            f32 roughness = 0.5f;
            f32 ao = 1.0f;
            f32 padding = 0.0f;

            std::shared_ptr<ITexture2D> albedoTexture;
            std::shared_ptr<ITexture2D> normalTexture;
            std::shared_ptr<ITexture2D> metallicRoughnessTexture;
            std::shared_ptr<ITexture2D> aoTexture;
            std::shared_ptr<ITexture2D> emissiveTexture;
        };

        /**
         * @brief Lighting data for shaders
         */
        struct LightingData
        {
            Math::Vec4 directionalLight = Math::Vec4(0.5f, -1.0f, 0.5f, 0.0f);
            Math::Vec4 lightColor = Math::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
            Math::Vec4 ambientColor = Math::Vec4(0.1f, 0.1f, 0.1f, 1.0f);
            f32 lightIntensity = 1.0f;
            f32 padding[3];
        };

        /**
         * @brief Main render system managing the entire rendering pipeline
         */
        class RenderSystem
        {
        public:
            RenderSystem();
            ~RenderSystem();

            // Initialization
            bool Initialize(IGraphicsDevice* device);
            void Shutdown();

            // Render pipeline
            void BeginFrame();
            void Render(const Scene& scene, const Camera& camera);
            void EndFrame();

            // Render pass management
            void AddRenderPass(std::shared_ptr<RenderPass> pass);
            void RemoveRenderPass(RenderPassType type);
            std::shared_ptr<RenderPass> GetRenderPass(RenderPassType type);

            // Resource management
            u32 CreateRenderTarget(const RenderTargetSpec& spec);
            std::shared_ptr<RenderTarget> GetRenderTarget(u32 id);

            // Configuration
            void SetVSync(bool enabled) { m_vsyncEnabled = enabled; }
            void SetMultisampling(bool enabled, u32 samples = 4);
            void EnableHDR(bool enabled) { m_hdrEnabled = enabled; }

            // Statistics
            struct RenderStats {
                u32 drawCalls = 0;
                u32 triangles = 0;
                u32 vertices = 0;
                f32 frameTime = 0.0f;
                f32 gpuTime = 0.0f;
            };
            const RenderStats& GetStats() const { return m_stats; }

        private:
            void SetupDefaultRenderPasses();
            void UpdateGlobalUniforms(const Camera& camera);

            IGraphicsDevice* m_device = nullptr;
            std::vector<std::shared_ptr<RenderPass>> m_renderPasses;
            std::unordered_map<u32, std::shared_ptr<RenderTarget>> m_renderTargets;
            
            // Command buffers
            std::unique_ptr<CommandBuffer> m_mainCommandBuffer;
            std::unique_ptr<CommandBuffer> m_shadowCommandBuffer;
            
            // Global uniform buffers
            std::shared_ptr<IUniformBuffer> m_cameraUBO;
            std::shared_ptr<IUniformBuffer> m_lightingUBO;
            
            // Configuration
            bool m_vsyncEnabled = true;
            bool m_hdrEnabled = false;
            bool m_multisamplingEnabled = false;
            u32 m_msaaSamples = 4;
            
            // Statistics
            RenderStats m_stats;
            u32 m_nextRenderTargetId = 1;
        };

    } // namespace Renderer
} // namespace Pyramid
