#include <Pyramid/Graphics/OpenGL/OpenGLDevice.hpp>
#include <Pyramid/Graphics/OpenGL/OpenGLStateManager.hpp>
#include <Pyramid/Graphics/OpenGL/OpenGLTexture.hpp>
#include <Pyramid/Graphics/OpenGL/Shader/OpenGLShader.hpp>
#include <Pyramid/Graphics/Material/Material.hpp>
#include <Pyramid/Graphics/Renderer/RenderSystem.hpp>
#include <Pyramid/Graphics/Texture/TextureCache.hpp>

#include "Fixtures/JPEGFixtures.hpp"
#include "TestGraphicsDevice.hpp"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace
{
    GLuint g_nextTexture = 10;
    GLenum g_lastInternalFormat = 0;
    GLenum g_lastDataFormat = 0;
    GLint g_lastMinFilter = 0;
    int g_generateMipmapCalls = 0;
    int g_subImageCalls = 0;
    std::vector<GLint> g_unpackAlignments;
    std::vector<GLuint> g_deletedTextures;
    GLenum g_activeTexture = GL_TEXTURE0;
    std::array<GLuint, 32> g_boundTextures{};
    std::vector<GLuint> g_textureBinds;
    std::vector<GLenum> g_textureBindUnits;

    int Fail(const char* message)
    {
        std::cerr << "TextureLoadingTests failure: " << message << '\n';
        return EXIT_FAILURE;
    }

    GLenum APIENTRY FakeGetError()
    {
        return GL_NO_ERROR;
    }

    void APIENTRY FakeGetIntegerv(GLenum name, GLint* value)
    {
        if (!value)
        {
            return;
        }
        switch (name)
        {
        case GL_UNPACK_ALIGNMENT: *value = 4; break;
        case GL_ACTIVE_TEXTURE: *value = static_cast<GLint>(g_activeTexture); break;
        case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS: *value = 32; break;
        case GL_DEPTH_FUNC: *value = GL_LESS; break;
        case GL_POLYGON_MODE:
            value[0] = GL_FILL;
            value[1] = GL_FILL;
            break;
        case GL_VIEWPORT:
        case GL_SCISSOR_BOX:
            value[0] = value[1] = 0;
            value[2] = 1280;
            value[3] = 720;
            break;
        default: *value = 0; break;
        }
    }

    GLboolean APIENTRY FakeIsEnabled(GLenum)
    {
        return GL_FALSE;
    }

    void APIENTRY FakeGetBooleanv(GLenum, GLboolean* value)
    {
        *value = GL_TRUE;
    }

    void APIENTRY FakeGetFloatv(GLenum, GLfloat* values)
    {
        values[0] = values[1] = values[2] = 0.0f;
        values[3] = 1.0f;
    }

    void APIENTRY FakeGetDoublev(GLenum, GLdouble* value)
    {
        *value = 1.0;
    }

    void APIENTRY FakeGenTextures(GLsizei count, GLuint* textures)
    {
        for (GLsizei index = 0; index < count; ++index)
        {
            textures[index] = g_nextTexture++;
        }
    }

    void APIENTRY FakeDeleteTextures(GLsizei count, const GLuint* textures)
    {
        g_deletedTextures.insert(g_deletedTextures.end(), textures, textures + count);
    }

    void APIENTRY FakeBindTexture(GLenum target, GLuint texture)
    {
        if (target == GL_TEXTURE_2D && g_activeTexture >= GL_TEXTURE0 &&
            g_activeTexture < GL_TEXTURE0 + g_boundTextures.size())
        {
            g_boundTextures[g_activeTexture - GL_TEXTURE0] = texture;
            g_textureBinds.push_back(texture);
            g_textureBindUnits.push_back(g_activeTexture);
        }
    }

    void APIENTRY FakePixelStorei(GLenum name, GLint value)
    {
        if (name == GL_UNPACK_ALIGNMENT)
        {
            g_unpackAlignments.push_back(value);
        }
    }

    void APIENTRY FakeTexImage2D(
        GLenum,
        GLint,
        GLint internalFormat,
        GLsizei,
        GLsizei,
        GLint,
        GLenum dataFormat,
        GLenum,
        const void*)
    {
        g_lastInternalFormat = static_cast<GLenum>(internalFormat);
        g_lastDataFormat = dataFormat;
    }

    void APIENTRY FakeTexSubImage2D(
        GLenum,
        GLint,
        GLint,
        GLint,
        GLsizei,
        GLsizei,
        GLenum,
        GLenum,
        const void*)
    {
        ++g_subImageCalls;
    }

    void APIENTRY FakeTexParameteri(GLenum, GLenum name, GLint value)
    {
        if (name == GL_TEXTURE_MIN_FILTER)
        {
            g_lastMinFilter = value;
        }
    }

    void APIENTRY FakeTexParameterfv(GLenum, GLenum, const GLfloat*)
    {
    }

    void APIENTRY FakeGenerateMipmap(GLenum)
    {
        ++g_generateMipmapCalls;
    }

    void APIENTRY FakeActiveTexture(GLenum texture)
    {
        g_activeTexture = texture;
    }

    void InstallFakeOpenGL()
    {
        glad_glGetError = FakeGetError;
        glad_glGetIntegerv = FakeGetIntegerv;
        glad_glIsEnabled = FakeIsEnabled;
        glad_glGetBooleanv = FakeGetBooleanv;
        glad_glGetFloatv = FakeGetFloatv;
        glad_glGetDoublev = FakeGetDoublev;
        glad_glGenTextures = FakeGenTextures;
        glad_glDeleteTextures = FakeDeleteTextures;
        glad_glBindTexture = FakeBindTexture;
        glad_glPixelStorei = FakePixelStorei;
        glad_glTexImage2D = FakeTexImage2D;
        glad_glTexSubImage2D = FakeTexSubImage2D;
        glad_glTexParameteri = FakeTexParameteri;
        glad_glTexParameterfv = FakeTexParameterfv;
        glad_glGenerateMipmap = FakeGenerateMipmap;
        glad_glActiveTexture = FakeActiveTexture;
    }

    class ForeignTexture final : public Pyramid::ITexture2D
    {
    public:
        explicit ForeignTexture(Pyramid::u32 rendererId) : m_rendererId(rendererId) {}
        void Bind(Pyramid::u32) const override {}
        void Unbind(Pyramid::u32) const override {}
        Pyramid::u32 GetWidth() const override { return 1; }
        Pyramid::u32 GetHeight() const override { return 1; }
        Pyramid::u32 GetRendererID() const override { return m_rendererId; }
        const std::string& GetPath() const override { return m_path; }
        bool IsLoaded() const override { return true; }

    private:
        Pyramid::u32 m_rendererId;
        std::string m_path;
    };

    class BindingShader final : public Pyramid::OpenGLShader
    {
    public:
        bool Compile(const std::string&, const std::string&) override { return true; }
        void Bind() override {}
        void SetUniformInt(const std::string& name, int value) override
        {
            uniformName = name;
            samplerSlot = value;
        }

        std::string uniformName;
        int samplerSlot = -1;
    };

    int TestDeviceTextureBinding()
    {
        using namespace Pyramid;

        OpenGLDevice device(nullptr);
        TextureSpecification specification;
        specification.GenerateMips = false;
        const std::array<u8, 4> pixel = {255, 127, 63, 255};
        auto direct = device.CreateTexture2D(specification, pixel.data());

        TextureCache cache(device);
        TextureResourceSpecification resourceSpecification;
        resourceSpecification.texture = specification;
        resourceSpecification.pixelData = pixel.data();
        resourceSpecification.pixelDataSize = pixel.size();
        auto cached = cache.GetOrCreate(resourceSpecification);
        if (!direct || !direct->IsLoaded() || !cached || !cached->IsLoaded() ||
            cached != cache.GetOrCreate(resourceSpecification))
        {
            return Fail("real OpenGL direct/cache texture setup failed");
        }
        TextureSpecification invalidSpecification = specification;
        invalidSpecification.Width = 0;
        OpenGLTexture2D unloaded(invalidSpecification, nullptr);
        if (unloaded.IsLoaded() || unloaded.GetRendererID() != 0)
        {
            return Fail("unloaded OpenGL texture fixture unexpectedly loaded");
        }

        auto foreign = std::make_shared<ForeignTexture>(cached->GetRendererID());
        auto shader = std::make_shared<BindingShader>();
        Tests::TestGraphicsDevice fixtureDevice;
        fixtureDevice.textureFactory = [&](const TextureSpecification&, const void*) { return foreign; };
        fixtureDevice.shaderFactory = [&]() { return shader; };
        auto foreignResource = TextureResource::Create(fixtureDevice, resourceSpecification);
        ShaderProgramSpecification shaderSpecification;
        shaderSpecification.vertexSource = "#version 330 core\nvoid main(){gl_Position=vec4(0.0);}";
        shaderSpecification.fragmentSource = "#version 330 core\nout vec4 color; void main(){color=vec4(1.0);}";
        auto program = ShaderProgram::Create(fixtureDevice, shaderSpecification);
        MaterialSpecification materialSpecification;
        materialSpecification.shader = program;
        materialSpecification.textures = {{"u_AlbedoMap", 4, cached}};
        materialSpecification.renderState.depthTest = false;
        materialSpecification.renderState.cullMode = MaterialCullMode::None;
        auto material = Material::Create(materialSpecification);
        if (!foreignResource || !program || !material)
        {
            return Fail("material or foreign-wrapper binding fixture setup failed");
        }

        OpenGLStateManager::GetInstance().InvalidateState();
        g_textureBinds.clear();
        g_textureBindUnits.clear();
        device.BindTexture(direct.get(), 3);
        if (g_textureBinds.size() != 1 || g_boundTextures[3] != direct->GetRendererID() ||
            g_textureBindUnits.back() != GL_TEXTURE3)
        {
            return Fail("OpenGLDevice did not bind a direct texture on the requested slot");
        }
        device.BindTexture(direct.get(), 3);
        if (g_textureBinds.size() != 1)
        {
            return Fail("repeated direct binding bypassed the OpenGL state cache");
        }
        device.BindTexture(cached.get(), 3);
        if (g_textureBinds.size() != 2 || g_boundTextures[3] != cached->GetRendererID() ||
            g_textureBindUnits.back() != GL_TEXTURE3 || !device.GetLastError().empty())
        {
            return Fail("OpenGLDevice rejected or failed to bind a cached TextureResource");
        }
        device.BindTexture(cached.get(), 3);
        device.BindTexture(cached->GetTexture().get(), 3);
        if (g_textureBinds.size() != 2)
        {
            return Fail("cached wrapper and its OpenGL texture did not share binding state");
        }

        Renderer::CommandBuffer commands;
        commands.RegisterTexture(7, cached.get());
        commands.Begin();
        commands.SetTexture(direct.get(), 6);
        commands.SetTexture(cached.get(), 6);
        commands.SetTexture(u32{7}, 7);
        commands.SetMaterial(material.get());
        commands.End();
        const auto priorBinds = g_textureBinds.size();
        commands.Execute(&device);
        if (g_textureBinds.size() != priorBinds + 4 ||
            g_boundTextures[6] != cached->GetRendererID() ||
            g_boundTextures[7] != cached->GetRendererID() ||
            g_boundTextures[4] != cached->GetRendererID() ||
            shader->uniformName != "u_AlbedoMap" || shader->samplerSlot != 4)
        {
            return Fail("command/material submission did not bind cached texture IDs and sampler slots");
        }

        device.BindTexture(nullptr, 3);
        device.BindTexture(&unloaded, 6);
        if (g_boundTextures[3] != 0 || g_boundTextures[6] != 0)
        {
            return Fail("null and unloaded textures no longer unbind their requested slots");
        }
        device.BindTexture(cached.get(), 31);
        if (g_boundTextures[31] != cached->GetRendererID())
        {
            return Fail("last supported texture slot was rejected");
        }
        const auto acceptedBinds = g_textureBinds.size();
        const auto acceptedUnit = g_activeTexture;
        device.BindTexture(cached.get(), 32);
        if (g_textureBinds.size() != acceptedBinds || g_activeTexture != acceptedUnit ||
            device.GetLastError().find("out of range") == std::string::npos)
        {
            return Fail("out-of-range texture binding changed GL state or lacked a diagnostic");
        }
        device.BindTexture(foreign.get(), 4);
        device.BindTexture(foreignResource.get(), 4);
        if (g_textureBinds.size() != acceptedBinds || g_activeTexture != acceptedUnit ||
            g_boundTextures[4] != cached->GetRendererID() ||
            device.GetLastError().find("invalid OpenGL texture") == std::string::npos)
        {
            return Fail("foreign texture or wrapper bypassed backend validation");
        }
        return EXIT_SUCCESS;
    }
}

int main()
{
    using Pyramid::OpenGLTexture2D;
    using Pyramid::TextureFormat;
    using Pyramid::Tests::Fixtures::BaselineRGBJPEG;
    using Pyramid::Tests::Fixtures::BaselineRGBJPEGSize;

    InstallFakeOpenGL();

    const std::string filepath = "pyramid_texture_fixture.jpg";
    {
        std::ofstream file(filepath, std::ios::binary);
        if (!file)
        {
            return Fail("could not create JPEG texture fixture");
        }
        file.write(
            reinterpret_cast<const char*>(BaselineRGBJPEG),
            static_cast<std::streamsize>(BaselineRGBJPEGSize));
    }

    GLuint textureId = 0;
    {
        OpenGLTexture2D texture(filepath, true, false);
        std::remove(filepath.c_str());

        if (!texture.IsLoaded() || texture.GetRendererID() == 0)
        {
            return Fail("valid JPEG texture did not load");
        }
        if (texture.GetWidth() != 8 || texture.GetHeight() != 8 ||
            texture.GetFormat() != TextureFormat::RGB8)
        {
            return Fail("loaded texture metadata is incorrect");
        }
        if (g_lastInternalFormat != GL_SRGB8 || g_lastDataFormat != GL_RGB)
        {
            return Fail("sRGB JPEG upload used incorrect OpenGL formats");
        }
        if (g_lastMinFilter != GL_LINEAR || g_generateMipmapCalls != 0)
        {
            return Fail("non-mipmapped texture used an incomplete minification state");
        }
        if (g_unpackAlignments.size() < 2 || g_unpackAlignments.front() != 1 ||
            g_unpackAlignments.back() != 4)
        {
            return Fail("texture upload did not set and restore unpack alignment");
        }

        textureId = texture.GetRendererID();
        if (texture.LoadFromFile("missing_texture.jpg", false, true))
        {
            return Fail("missing replacement texture unexpectedly loaded");
        }
        if (!texture.IsLoaded() || texture.GetRendererID() != textureId ||
            texture.GetLastError().empty())
        {
            return Fail("failed reload did not preserve the previous valid texture");
        }

        std::vector<unsigned char> pixels(8U * 8U * 3U, 127);
        texture.SetData(pixels.data(), static_cast<Pyramid::u32>(pixels.size() - 1));
        if (texture.GetLastError().empty() || g_subImageCalls != 0)
        {
            return Fail("invalid SetData size was not rejected");
        }

        texture.SetData(pixels.data(), static_cast<Pyramid::u32>(pixels.size()));
        if (!texture.GetLastError().empty() || g_subImageCalls != 1)
        {
            return Fail("valid SetData update failed");
        }
    }

    if (std::find(g_deletedTextures.begin(), g_deletedTextures.end(), textureId) ==
        g_deletedTextures.end())
    {
        return Fail("texture object was not released");
    }

    if (TestDeviceTextureBinding() != EXIT_SUCCESS)
    {
        return EXIT_FAILURE;
    }

    std::cout << "Texture loading tests passed\n";
    return EXIT_SUCCESS;
}
