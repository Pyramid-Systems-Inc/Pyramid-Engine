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
            DeferredGeometryPass();
            ~DeferredGeometryPass() override = default;

            void Begin(CommandBuffer& cmd) override;
            void Execute(CommandBuffer& cmd, const Scene& scene, const Camera& camera) override;
            void End(CommandBuffer& cmd) override;

            // G-Buffer layout:
            // RT0: Albedo (RGB) + Metallic (A)
            // RT1: Normal (RGB) + Roughness (A)
            // RT2: Motion Vectors (RG) + AO (B) + Material ID (A)
            // RT3: Emissive (RGB) + Flags (A)
            // Depth: Depth + Stencil

        private:
            void SetupGBufferShaders();
        };

        /**
         * @brief Deferred lighting pass
         */
        class DeferredLightingPass : public RenderPass
        {
        public:
            DeferredLightingPass();
            ~DeferredLightingPass() override = default;

            void Begin(CommandBuffer& cmd) override;
            void Execute(CommandBuffer& cmd, const Scene& scene, const Camera& camera) override;
            void End(CommandBuffer& cmd) override;

            // Lighting techniques
            void SetAmbientOcclusion(bool enabled) { m_ssaoEnabled = enabled; }
            void SetImageBasedLighting(bool enabled) { m_iblEnabled = enabled; }

        private:
            bool m_ssaoEnabled = false;
            bool m_iblEnabled = false;
        };

        /**
         * @brief Shadow mapping pass
         */
        class ShadowMapPass : public RenderPass
        {
        public:
            ShadowMapPass();
            ~ShadowMapPass() override = default;

            void Begin(CommandBuffer& cmd) override;
            void Execute(CommandBuffer& cmd, const Scene& scene, const Camera& camera) override;
            void End(CommandBuffer& cmd) override;

            // Configuration
            void SetShadowMapSize(u32 size) { m_shadowMapSize = size; }
            void SetCascadeCount(u32 count) { m_cascadeCount = count; }
            void SetShadowBias(f32 bias) { m_shadowBias = bias; }

            // Accessors
            u32 GetShadowMapTexture() const { return m_shadowMapTexture; }

        private:
            void SetupShadowMapRenderTarget();
            void CalculateCascadeSplits(const Camera& camera);

            u32 m_shadowMapSize = 2048;
            u32 m_cascadeCount = 4;
            f32 m_shadowBias = 0.005f;
            u32 m_shadowMapTexture = 0;
            std::vector<f32> m_cascadeSplits;
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
