#pragma once

#include <Pyramid/Graphics/Renderer/RenderSystem.hpp>
#include <Pyramid/Math/Math.hpp>

namespace Pyramid
{
    namespace Renderer
    {

        /**
         * @brief Forward rendering pass for opaque geometry
         */
        class ForwardRenderPass : public RenderPass
        {
        public:
            ForwardRenderPass();
            ~ForwardRenderPass() override = default;

            void Begin(CommandBuffer& cmd) override;
            void Execute(CommandBuffer& cmd, const Scene& scene, const Camera& camera) override;
            void End(CommandBuffer& cmd) override;

            // Configuration
            void SetClearColor(const Math::Vec4& color) { m_clearColor = color; }
            void SetWireframeMode(bool enabled) { m_wireframe = enabled; }

        private:
            Math::Vec4 m_clearColor = Math::Vec4(0.2f, 0.3f, 0.3f, 1.0f);
            bool m_wireframe = false;
        };

        /**
         * @brief Deferred geometry pass (G-Buffer generation)
         */
        class DeferredGeometryPass : public RenderPass
        {
        public:
            DeferredGeometryPass(const std::string& name, IGraphicsDevice* device, u32 width, u32 height);
            ~DeferredGeometryPass() override = default;

            void Begin(CommandBuffer& cmd) override;
            void Execute(CommandBuffer& cmd, const Scene& scene, const Camera& camera) override;
            void End(CommandBuffer& cmd) override;

            void Resize(u32 width, u32 height);
            
            std::shared_ptr<class OpenGLFramebuffer> GetGBuffer() const { return m_gBuffer; }

            // G-Buffer layout:
            // RT0: Albedo (RGB) + Metallic (A)      - GL_RGBA8
            // RT1: Normal (RGB) + Roughness (A)     - GL_RGBA16F
            // RT2: WorldPos (RGB) + AO (A)          - GL_RGBA16F
            // RT3: Emissive (RGB) + Flags (A)       - GL_RGBA16F
            // Depth: Depth24Stencil8

        private:
            void CreateGBuffer();
            
            IGraphicsDevice* m_device;
            u32 m_width;
            u32 m_height;
            std::shared_ptr<class OpenGLFramebuffer> m_gBuffer;
            std::shared_ptr<IShader> m_geometryShader;
        };

        /**
         * @brief Deferred lighting pass
         */
        class DeferredLightingPass : public RenderPass
        {
        public:
            DeferredLightingPass(const std::string& name, IGraphicsDevice* device);
            ~DeferredLightingPass() override = default;

            void Begin(CommandBuffer& cmd) override;
            void Execute(CommandBuffer& cmd, const Scene& scene, const Camera& camera) override;
            void End(CommandBuffer& cmd) override;

            void SetGBuffer(std::shared_ptr<class OpenGLFramebuffer> gBuffer);
            void SetShadowMaps(const std::vector<std::shared_ptr<class OpenGLFramebuffer>>& shadowMaps);
            void SetEnableSSAO(bool enable) { m_enableSSAO = enable; }
            void SetEnableIBL(bool enable) { m_enableIBL = enable; }

        private:
            void CreateFullscreenQuad();
            
            IGraphicsDevice* m_device;
            std::shared_ptr<class OpenGLFramebuffer> m_gBuffer;
            std::vector<std::shared_ptr<class OpenGLFramebuffer>> m_shadowMaps;
            std::shared_ptr<IShader> m_lightingShader;
            std::shared_ptr<class IVertexArray> m_fullscreenQuad;
            bool m_enableSSAO = false;
            bool m_enableIBL = false;
        };

        /**
         * @brief Shadow mapping pass with cascaded shadow maps
         */
        class ShadowMapPass : public RenderPass
        {
        public:
            ShadowMapPass(const std::string& name, IGraphicsDevice* device, u32 cascadeCount = 4);
            ~ShadowMapPass() override = default;

            void Begin(CommandBuffer& cmd) override;
            void Execute(CommandBuffer& cmd, const Scene& scene, const Camera& camera) override;
            void End(CommandBuffer& cmd) override;

            // Configuration
            void SetCascadeCount(u32 count);
            void SetShadowMapResolution(u32 resolution);
            void SetCascadeSplits(const std::vector<f32>& splits);
            void SetDepthBias(f32 bias);

            // Accessors
            const std::vector<std::shared_ptr<class OpenGLFramebuffer>>& GetShadowMaps() const { return m_shadowMaps; }
            const std::vector<Math::Mat4>& GetLightSpaceMatrices() const { return m_lightSpaceMatrices; }

        private:
            void CreateShadowMaps();
            void CalculateCascadeSplits(const Camera& camera);
            Math::Mat4 CalculateLightSpaceMatrix(const Camera& camera, f32 nearPlane, f32 farPlane, const Math::Vec3& lightDir);

            IGraphicsDevice* m_device;
            u32 m_cascadeCount;
            u32 m_shadowMapResolution;
            std::vector<f32> m_cascadeSplits;
            std::vector<std::shared_ptr<class OpenGLFramebuffer>> m_shadowMaps;
            std::vector<Math::Mat4> m_lightSpaceMatrices;
            f32 m_depthBias;
            std::shared_ptr<IShader> m_shadowShader;
        };

        /**
         * @brief Transparent objects rendering pass
         */
        class TransparentPass : public RenderPass
        {
        public:
            TransparentPass();
            ~TransparentPass() override = default;

            void Begin(CommandBuffer& cmd) override;
            void Execute(CommandBuffer& cmd, const Scene& scene, const Camera& camera) override;
            void End(CommandBuffer& cmd) override;

            // Transparency techniques
            enum class TransparencyMode {
                AlphaBlend,     // Traditional alpha blending
                AlphaToCoverage, // MSAA-based transparency
                OrderIndependent // OIT techniques
            };

            void SetTransparencyMode(TransparencyMode mode) { m_transparencyMode = mode; }

        private:
            TransparencyMode m_transparencyMode = TransparencyMode::AlphaBlend;
        };

        /**
         * @brief Post-processing pass
         */
        class PostProcessPass : public RenderPass
        {
        public:
            PostProcessPass();
            ~PostProcessPass() override = default;

            void Begin(CommandBuffer& cmd) override;
            void Execute(CommandBuffer& cmd, const Scene& scene, const Camera& camera) override;
            void End(CommandBuffer& cmd) override;

            // Post-processing effects
            struct PostProcessSettings {
                // Tone mapping
                bool enableToneMapping = true;
                f32 exposure = 1.0f;
                f32 gamma = 2.2f;

                // Color grading
                bool enableColorGrading = false;
                f32 contrast = 1.0f;
                f32 saturation = 1.0f;
                f32 brightness = 0.0f;

                // Anti-aliasing
                bool enableFXAA = true;
                bool enableTAA = false;

                // Bloom
                bool enableBloom = false;
                f32 bloomThreshold = 1.0f;
                f32 bloomIntensity = 0.5f;

                // Screen-space effects
                bool enableSSAO = false;
                bool enableSSR = false;
            };

            void SetSettings(const PostProcessSettings& settings) { m_settings = settings; }
            const PostProcessSettings& GetSettings() const { return m_settings; }

        private:
            void ApplyToneMapping(CommandBuffer& cmd);
            void ApplyFXAA(CommandBuffer& cmd);
            void ApplyBloom(CommandBuffer& cmd);

            PostProcessSettings m_settings;
        };

        /**
         * @brief UI/GUI rendering pass
         */
        class UIRenderPass : public RenderPass
        {
        public:
            UIRenderPass();
            ~UIRenderPass() override = default;

            void Begin(CommandBuffer& cmd) override;
            void Execute(CommandBuffer& cmd, const Scene& scene, const Camera& camera) override;
            void End(CommandBuffer& cmd) override;

            // UI rendering modes
            enum class UIMode {
                Immediate,  // Immediate mode GUI (like Dear ImGui)
                Retained    // Retained mode GUI with widget hierarchy
            };

            void SetUIMode(UIMode mode) { m_uiMode = mode; }

        private:
            UIMode m_uiMode = UIMode::Immediate;
        };

        /**
         * @brief Debug visualization pass
         */
        class DebugRenderPass : public RenderPass
        {
        public:
            DebugRenderPass();
            ~DebugRenderPass() override = default;

            void Begin(CommandBuffer& cmd) override;
            void Execute(CommandBuffer& cmd, const Scene& scene, const Camera& camera) override;
            void End(CommandBuffer& cmd) override;

            // Debug visualization options
            struct DebugSettings {
                bool showWireframe = false;
                bool showNormals = false;
                bool showTangents = false;
                bool showBoundingBoxes = false;
                bool showLightVolumes = false;
                bool showShadowCascades = false;
                bool showGBuffer = false;
                u32 gBufferChannel = 0; // 0=albedo, 1=normal, 2=roughness, etc.
            };

            void SetSettings(const DebugSettings& settings) { m_settings = settings; }
            const DebugSettings& GetSettings() const { return m_settings; }

        private:
            DebugSettings m_settings;
        };

        /**
         * @brief Factory for creating render passes
         */
        class RenderPassFactory
        {
        public:
            static std::shared_ptr<RenderPass> CreateForwardPass();
            static std::shared_ptr<RenderPass> CreateDeferredGeometryPass();
            static std::shared_ptr<RenderPass> CreateDeferredLightingPass();
            static std::shared_ptr<RenderPass> CreateShadowMapPass();
            static std::shared_ptr<RenderPass> CreateTransparentPass();
            static std::shared_ptr<RenderPass> CreatePostProcessPass();
            static std::shared_ptr<RenderPass> CreateUIPass();
            static std::shared_ptr<RenderPass> CreateDebugPass();

            // Preset configurations
            static std::vector<std::shared_ptr<RenderPass>> CreateForwardPipeline();
            static std::vector<std::shared_ptr<RenderPass>> CreateDeferredPipeline();
            static std::vector<std::shared_ptr<RenderPass>> CreateHDRPipeline();
        };

    } // namespace Renderer
} // namespace Pyramid
