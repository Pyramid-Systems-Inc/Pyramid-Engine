#include <Pyramid/Graphics/Renderer/RenderPasses.hpp>
#include <Pyramid/Graphics/GraphicsDevice.hpp>
#include <Pyramid/Graphics/Scene.hpp>
#include <Pyramid/Graphics/Camera.hpp>
#include <Pyramid/Graphics/Shader/Shader.hpp>
#include <Pyramid/Graphics/Texture.hpp>
#include <Pyramid/Graphics/Buffer/VertexArray.hpp>
#include <Pyramid/Util/Log.hpp>
#include <glad/glad.h>

namespace Pyramid
{
    namespace Renderer
    {

        ForwardRenderPass::ForwardRenderPass()
            : RenderPass(RenderPassType::Forward, "ForwardRenderPass")
        {
            PYRAMID_LOG_INFO("ForwardRenderPass created");
        }

        void ForwardRenderPass::Begin(CommandBuffer& cmd)
        {
            // Clear color and depth buffers
            cmd.ClearTarget(m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w);
            
            // Set wireframe mode if enabled
            if (m_wireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            } else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            
            PYRAMID_LOG_DEBUG("ForwardRenderPass::Begin - Clear color: ", 
                m_clearColor.x, ", ", m_clearColor.y, ", ", m_clearColor.z, ", ", m_clearColor.w);
        }

        void ForwardRenderPass::Execute(CommandBuffer& cmd, const Scene& scene, const Camera& camera)
        {
            // Get visible objects from the scene
            auto visibleObjects = scene.GetVisibleObjects(camera);
            
            PYRAMID_LOG_DEBUG("ForwardRenderPass::Execute - Rendering ", visibleObjects.size(), " visible objects");
            
            // Render each visible object
            for (const auto& object : visibleObjects)
            {
                if (!object || !object->visible) continue;
                
                // Skip objects without geometry
                if (!object->vertexArray) {
                    PYRAMID_LOG_WARN("Object '", object->name, "' has no vertex array, skipping");
                    continue;
                }
                
                // Bind shader if available
                if (object->material.shader)
                {
                    object->material.shader->Bind();
                    
                    // Calculate matrices
                    Math::Mat4 model = object->GetTransformMatrix();
                    Math::Mat4 viewProj = camera.GetViewProjectionMatrix();
                    
                    // Set per-object uniforms
                    object->material.shader->SetUniformMat4("u_Model", model.m);
                    object->material.shader->SetUniformMat4("u_ViewProjection", viewProj.m);
                    
                    // Calculate normal matrix (inverse transpose of upper-left 3x3)
                    Math::Mat4 normalMatrix = model.Inverse().Transpose();
                    object->material.shader->SetUniformMat4("u_NormalMatrix", normalMatrix.m);
                    
                    // Set material albedo color
                    object->material.shader->SetUniformFloat4("u_AlbedoColor", 
                        object->material.albedo.x, 
                        object->material.albedo.y, 
                        object->material.albedo.z, 
                        object->material.albedo.w);
                    
                    // Bind albedo texture if available
                    if (object->material.albedoTexture)
                    {
                        object->material.albedoTexture->Bind(0);
                        object->material.shader->SetUniformInt("u_AlbedoMap", 0);
                        object->material.shader->SetUniformInt("u_HasAlbedoMap", 1);
                    }
                    else
                    {
                        object->material.shader->SetUniformInt("u_HasAlbedoMap", 0);
                    }
                    
                    // Bind normal texture if available
                    if (object->material.normalTexture)
                    {
                        object->material.normalTexture->Bind(1);
                        object->material.shader->SetUniformInt("u_NormalMap", 1);
                    }
                    
                    // Bind metallic-roughness texture if available
                    if (object->material.metallicRoughnessTexture)
                    {
                        object->material.metallicRoughnessTexture->Bind(2);
                        object->material.shader->SetUniformInt("u_MetallicRoughnessMap", 2);
                    }
                }
                else
                {
                    PYRAMID_LOG_WARN("Object '", object->name, "' has no shader, skipping");
                    continue;
                }
                
                // Bind vertex array
                object->vertexArray->Bind();
                
                // Get index count and issue draw call
                u32 indexCount = object->vertexArray->GetIndexBuffer()->GetCount();
                cmd.DrawIndexed(indexCount);
            }
        }

        void ForwardRenderPass::End(CommandBuffer& cmd)
        {
            // Reset wireframe mode to fill
            if (m_wireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            
            PYRAMID_LOG_DEBUG("ForwardRenderPass::End");
        }

    } // namespace Renderer
} // namespace Pyramid