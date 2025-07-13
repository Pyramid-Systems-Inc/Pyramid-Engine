#include <Pyramid/Graphics/OpenGL/OpenGLStateManager.hpp>
#include <Pyramid/Util/Log.hpp>

namespace Pyramid
{

    OpenGLStateManager &OpenGLStateManager::GetInstance()
    {
        static OpenGLStateManager instance;
        if (!instance.m_initialized)
        {
            instance.InitializeState();
        }
        return instance;
    }

    void OpenGLStateManager::InitializeState()
    {
        // Query current OpenGL state to initialize cache
        glGetIntegerv(GL_CURRENT_PROGRAM, reinterpret_cast<GLint *>(&m_currentProgram));
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, reinterpret_cast<GLint *>(&m_currentVAO));
        glGetIntegerv(GL_ACTIVE_TEXTURE, reinterpret_cast<GLint *>(&m_activeTexture));

        // Initialize buffer bindings
        GLint arrayBuffer, elementArrayBuffer;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &arrayBuffer);
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &elementArrayBuffer);
        m_boundBuffers[GL_ARRAY_BUFFER] = arrayBuffer;
        m_boundBuffers[GL_ELEMENT_ARRAY_BUFFER] = elementArrayBuffer;

        // Initialize framebuffer bindings
        GLint drawFramebuffer, readFramebuffer;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFramebuffer);
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFramebuffer);
        m_boundFramebuffers[GL_DRAW_FRAMEBUFFER] = drawFramebuffer;
        m_boundFramebuffers[GL_READ_FRAMEBUFFER] = readFramebuffer;

        // Initialize viewport
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        m_viewport.x = viewport[0];
        m_viewport.y = viewport[1];
        m_viewport.width = viewport[2];
        m_viewport.height = viewport[3];

        // Initialize blend state
        m_blendState.enabled = glIsEnabled(GL_BLEND);
        glGetIntegerv(GL_BLEND_SRC_RGB, reinterpret_cast<GLint *>(&m_blendState.srcRGB));
        glGetIntegerv(GL_BLEND_DST_RGB, reinterpret_cast<GLint *>(&m_blendState.dstRGB));
        glGetIntegerv(GL_BLEND_SRC_ALPHA, reinterpret_cast<GLint *>(&m_blendState.srcAlpha));
        glGetIntegerv(GL_BLEND_DST_ALPHA, reinterpret_cast<GLint *>(&m_blendState.dstAlpha));
        glGetIntegerv(GL_BLEND_EQUATION_RGB, reinterpret_cast<GLint *>(&m_blendState.equation));

        // Initialize depth state
        m_depthState.testEnabled = glIsEnabled(GL_DEPTH_TEST);
        glGetIntegerv(GL_DEPTH_FUNC, reinterpret_cast<GLint *>(&m_depthState.func));
        GLboolean depthMask;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
        m_depthState.maskEnabled = depthMask;

        // Initialize stencil state
        m_stencilState.testEnabled = glIsEnabled(GL_STENCIL_TEST);
        glGetIntegerv(GL_STENCIL_FUNC, reinterpret_cast<GLint *>(&m_stencilState.func));
        glGetIntegerv(GL_STENCIL_REF, &m_stencilState.ref);
        glGetIntegerv(GL_STENCIL_VALUE_MASK, reinterpret_cast<GLint *>(&m_stencilState.mask));

        // Initialize cull state
        m_cullState.enabled = glIsEnabled(GL_CULL_FACE);
        glGetIntegerv(GL_CULL_FACE_MODE, reinterpret_cast<GLint *>(&m_cullState.mode));
        glGetIntegerv(GL_FRONT_FACE, reinterpret_cast<GLint *>(&m_cullState.frontFace));

        // Initialize scissor state
        m_scissorState.enabled = glIsEnabled(GL_SCISSOR_TEST);
        GLint scissorBox[4];
        glGetIntegerv(GL_SCISSOR_BOX, scissorBox);
        m_scissorState.x = scissorBox[0];
        m_scissorState.y = scissorBox[1];
        m_scissorState.width = scissorBox[2];
        m_scissorState.height = scissorBox[3];

        // Initialize clear state
        GLfloat clearColor[4];
        glGetFloatv(GL_COLOR_CLEAR_VALUE, clearColor);
        m_clearState.colorR = clearColor[0];
        m_clearState.colorG = clearColor[1];
        m_clearState.colorB = clearColor[2];
        m_clearState.colorA = clearColor[3];
        glGetDoublev(GL_DEPTH_CLEAR_VALUE, &m_clearState.depth);
        glGetIntegerv(GL_STENCIL_CLEAR_VALUE, &m_clearState.stencil);

        m_initialized = true;
        PYRAMID_LOG_INFO("OpenGL state manager initialized");
    }

    void OpenGLStateManager::UseProgram(GLuint program)
    {
        if (m_currentProgram != program)
        {
            glUseProgram(program);
            m_currentProgram = program;
            m_stateChangeCount++;
        }
    }

    void OpenGLStateManager::BindVertexArray(GLuint vao)
    {
        if (m_currentVAO != vao)
        {
            glBindVertexArray(vao);
            m_currentVAO = vao;
            m_stateChangeCount++;
        }
    }

    void OpenGLStateManager::BindBuffer(GLenum target, GLuint buffer)
    {
        auto it = m_boundBuffers.find(target);
        if (it == m_boundBuffers.end() || it->second != buffer)
        {
            glBindBuffer(target, buffer);
            m_boundBuffers[target] = buffer;
            m_stateChangeCount++;
        }
    }

    GLuint OpenGLStateManager::GetBoundBuffer(GLenum target) const
    {
        auto it = m_boundBuffers.find(target);
        return (it != m_boundBuffers.end()) ? it->second : 0;
    }

    void OpenGLStateManager::ActiveTexture(GLenum texture)
    {
        if (m_activeTexture != texture)
        {
            glActiveTexture(texture);
            m_activeTexture = texture;
            m_stateChangeCount++;
        }
    }

    void OpenGLStateManager::BindTexture(GLenum target, GLuint texture)
    {
        u32 unit = m_activeTexture - GL_TEXTURE0;
        if (unit < MAX_TEXTURE_UNITS)
        {
            auto &unitTextures = m_boundTextures[unit];
            auto it = unitTextures.find(target);
            if (it == unitTextures.end() || it->second != texture)
            {
                glBindTexture(target, texture);
                unitTextures[target] = texture;
                m_stateChangeCount++;
            }
        }
        else
        {
            // Fallback for invalid texture unit
            glBindTexture(target, texture);
            m_stateChangeCount++;
        }
    }

    void OpenGLStateManager::BindTextureUnit(u32 unit, GLenum target, GLuint texture)
    {
        if (unit < MAX_TEXTURE_UNITS)
        {
            ActiveTexture(GL_TEXTURE0 + unit);
            BindTexture(target, texture);
        }
    }

    void OpenGLStateManager::BindFramebuffer(GLenum target, GLuint framebuffer)
    {
        auto it = m_boundFramebuffers.find(target);
        if (it == m_boundFramebuffers.end() || it->second != framebuffer)
        {
            glBindFramebuffer(target, framebuffer);
            m_boundFramebuffers[target] = framebuffer;
            m_stateChangeCount++;
        }
    }

    GLuint OpenGLStateManager::GetBoundFramebuffer(GLenum target) const
    {
        auto it = m_boundFramebuffers.find(target);
        return (it != m_boundFramebuffers.end()) ? it->second : 0;
    }

    void OpenGLStateManager::SetViewport(i32 x, i32 y, i32 width, i32 height)
    {
        if (m_viewport.x != x || m_viewport.y != y ||
            m_viewport.width != width || m_viewport.height != height)
        {
            glViewport(x, y, width, height);
            m_viewport.x = x;
            m_viewport.y = y;
            m_viewport.width = width;
            m_viewport.height = height;
            m_stateChangeCount++;
        }
    }

    void OpenGLStateManager::GetViewport(i32 &x, i32 &y, i32 &width, i32 &height) const
    {
        x = m_viewport.x;
        y = m_viewport.y;
        width = m_viewport.width;
        height = m_viewport.height;
    }

    void OpenGLStateManager::EnableBlend(bool enable)
    {
        if (m_blendState.enabled != enable)
        {
            if (enable)
                glEnable(GL_BLEND);
            else
                glDisable(GL_BLEND);
            m_blendState.enabled = enable;
            m_stateChangeCount++;
        }
    }

    void OpenGLStateManager::SetBlendFunc(GLenum sfactor, GLenum dfactor)
    {
        if (m_blendState.srcRGB != sfactor || m_blendState.dstRGB != dfactor ||
            m_blendState.srcAlpha != sfactor || m_blendState.dstAlpha != dfactor)
        {
            glBlendFunc(sfactor, dfactor);
            m_blendState.srcRGB = sfactor;
            m_blendState.dstRGB = dfactor;
            m_blendState.srcAlpha = sfactor;
            m_blendState.dstAlpha = dfactor;
            m_stateChangeCount++;
        }
    }

    void OpenGLStateManager::SetBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
    {
        if (m_blendState.srcRGB != srcRGB || m_blendState.dstRGB != dstRGB ||
            m_blendState.srcAlpha != srcAlpha || m_blendState.dstAlpha != dstAlpha)
        {
            glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
            m_blendState.srcRGB = srcRGB;
            m_blendState.dstRGB = dstRGB;
            m_blendState.srcAlpha = srcAlpha;
            m_blendState.dstAlpha = dstAlpha;
            m_stateChangeCount++;
        }
    }

    void OpenGLStateManager::SetBlendEquation(GLenum mode)
    {
        if (m_blendState.equation != mode)
        {
            glBlendEquation(mode);
            m_blendState.equation = mode;
            m_stateChangeCount++;
        }
    }

    void OpenGLStateManager::EnableDepthTest(bool enable)
    {
        if (m_depthState.testEnabled != enable)
        {
            if (enable)
                glEnable(GL_DEPTH_TEST);
            else
                glDisable(GL_DEPTH_TEST);
            m_depthState.testEnabled = enable;
            m_stateChangeCount++;
        }
    }

    void OpenGLStateManager::SetDepthFunc(GLenum func)
    {
        if (m_depthState.func != func)
        {
            glDepthFunc(func);
            m_depthState.func = func;
            m_stateChangeCount++;
        }
    }

    void OpenGLStateManager::SetDepthMask(bool flag)
    {
        if (m_depthState.maskEnabled != flag)
        {
            glDepthMask(flag ? GL_TRUE : GL_FALSE);
            m_depthState.maskEnabled = flag;
            m_stateChangeCount++;
        }
    }

    void OpenGLStateManager::EnableStencilTest(bool enable)
    {
        if (m_stencilState.testEnabled != enable)
        {
            if (enable)
                glEnable(GL_STENCIL_TEST);
            else
                glDisable(GL_STENCIL_TEST);
            m_stencilState.testEnabled = enable;
            m_stateChangeCount++;
        }
    }

    void OpenGLStateManager::SetStencilFunc(GLenum func, i32 ref, u32 mask)
    {
        if (m_stencilState.func != func || m_stencilState.ref != ref || m_stencilState.mask != mask)
        {
            glStencilFunc(func, ref, mask);
            m_stencilState.func = func;
            m_stencilState.ref = ref;
            m_stencilState.mask = mask;
            m_stateChangeCount++;
        }
    }

    void OpenGLStateManager::SetStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
    {
        if (m_stencilState.fail != fail || m_stencilState.zfail != zfail || m_stencilState.zpass != zpass)
        {
            glStencilOp(fail, zfail, zpass);
            m_stencilState.fail = fail;
            m_stencilState.zfail = zfail;
            m_stencilState.zpass = zpass;
            m_stateChangeCount++;
        }
    }

    void OpenGLStateManager::EnableCullFace(bool enable)
    {
        if (m_cullState.enabled != enable)
        {
            if (enable)
                glEnable(GL_CULL_FACE);
            else
                glDisable(GL_CULL_FACE);
            m_cullState.enabled = enable;
            m_stateChangeCount++;
        }
    }

    void OpenGLStateManager::SetCullFace(GLenum mode)
    {
        if (m_cullState.mode != mode)
        {
            glCullFace(mode);
            m_cullState.mode = mode;
            m_stateChangeCount++;
        }
    }

    void OpenGLStateManager::SetFrontFace(GLenum mode)
    {
        if (m_cullState.frontFace != mode)
        {
            glFrontFace(mode);
            m_cullState.frontFace = mode;
            m_stateChangeCount++;
        }
    }

    void OpenGLStateManager::EnableScissorTest(bool enable)
    {
        if (m_scissorState.enabled != enable)
        {
            if (enable)
                glEnable(GL_SCISSOR_TEST);
            else
                glDisable(GL_SCISSOR_TEST);
            m_scissorState.enabled = enable;
            m_stateChangeCount++;
        }
    }

    void OpenGLStateManager::SetScissor(i32 x, i32 y, i32 width, i32 height)
    {
        if (m_scissorState.x != x || m_scissorState.y != y ||
            m_scissorState.width != width || m_scissorState.height != height)
        {
            glScissor(x, y, width, height);
            m_scissorState.x = x;
            m_scissorState.y = y;
            m_scissorState.width = width;
            m_scissorState.height = height;
            m_stateChangeCount++;
        }
    }

    void OpenGLStateManager::SetClearColor(f32 r, f32 g, f32 b, f32 a)
    {
        if (m_clearState.colorR != r || m_clearState.colorG != g ||
            m_clearState.colorB != b || m_clearState.colorA != a)
        {
            glClearColor(r, g, b, a);
            m_clearState.colorR = r;
            m_clearState.colorG = g;
            m_clearState.colorB = b;
            m_clearState.colorA = a;
            m_stateChangeCount++;
        }
    }

    void OpenGLStateManager::SetClearDepth(f64 depth)
    {
        if (m_clearState.depth != depth)
        {
            glClearDepth(depth);
            m_clearState.depth = depth;
            m_stateChangeCount++;
        }
    }

    void OpenGLStateManager::SetClearStencil(i32 stencil)
    {
        if (m_clearState.stencil != stencil)
        {
            glClearStencil(stencil);
            m_clearState.stencil = stencil;
            m_stateChangeCount++;
        }
    }

    void OpenGLStateManager::InvalidateState()
    {
        // Force refresh of all cached state on next access
        m_initialized = false;
        PYRAMID_LOG_INFO("OpenGL state cache invalidated");
    }

    void OpenGLStateManager::ResetToDefaults()
    {
        // Reset to OpenGL default state
        UseProgram(0);
        BindVertexArray(0);
        BindBuffer(GL_ARRAY_BUFFER, 0);
        BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        BindFramebuffer(GL_FRAMEBUFFER, 0);

        EnableBlend(false);
        SetBlendFunc(GL_ONE, GL_ZERO);
        SetBlendEquation(GL_FUNC_ADD);

        EnableDepthTest(false);
        SetDepthFunc(GL_LESS);
        SetDepthMask(true);

        EnableStencilTest(false);
        SetStencilFunc(GL_ALWAYS, 0, 0xFFFFFFFF);
        SetStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        EnableCullFace(false);
        SetCullFace(GL_BACK);
        SetFrontFace(GL_CCW);

        EnableScissorTest(false);

        SetClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        SetClearDepth(1.0);
        SetClearStencil(0);

        PYRAMID_LOG_INFO("OpenGL state reset to defaults");
    }

} // namespace Pyramid
