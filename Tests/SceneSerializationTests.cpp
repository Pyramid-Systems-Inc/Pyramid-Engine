#include <Pyramid/Graphics/Geometry/Vertex.hpp>
#include <Pyramid/Graphics/Resources/ResourceManifest.hpp>
#include <Pyramid/Graphics/Resources/ResourceRegistry.hpp>
#include <Pyramid/Graphics/Scene.hpp>
#include <Pyramid/Graphics/Scene/SceneSerializer.hpp>

#include "TestGraphicsDevice.hpp"

#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

namespace
{
    using namespace Pyramid;

    class TestShader final : public IShader
    {
    public:
        void Bind() override {}
        void Unbind() override {}
        bool Compile(const std::string&, const std::string&) override { return true; }
        bool CompileWithGeometry(const std::string&, const std::string&, const std::string&) override { return true; }
        bool CompileWithTessellation(const std::string&, const std::string&, const std::string&, const std::string&) override { return true; }
        bool CompileAdvanced(const std::string&, const std::string&, const std::string&, const std::string&, const std::string&) override { return true; }
        bool CompileCompute(const std::string&) override { return true; }
        void DispatchCompute(u32, u32, u32) override {}
        void SetUniformInt(const std::string&, int) override {}
        void SetUniformFloat(const std::string&, float) override {}
        void SetUniformFloat2(const std::string&, float, float) override {}
        void SetUniformFloat3(const std::string&, float, float, float) override {}
        void SetUniformFloat4(const std::string&, float, float, float, float) override {}
        void SetUniformMat3(const std::string&, const float*, bool, int) override {}
        void SetUniformMat4(const std::string&, const float*, bool, int) override {}
        void BindUniformBuffer(const std::string&, IUniformBuffer*, u32) override {}
        void SetUniformBlockBinding(const std::string&, u32) override {}
        void BindShaderStorageBuffer(const std::string&, IShaderStorageBuffer*, u32) override {}
        void SetShaderStorageBlockBinding(const std::string&, u32) override {}
    };

    int Fail(const char* message)
    {
        std::cerr << "SceneSerializationTests failure: " << message << '\n';
        return EXIT_FAILURE;
    }

    bool NearlyEqual(float left, float right, float epsilon = 0.0001f)
    {
        return std::fabs(left - right) <= epsilon;
    }

    bool NearlyEqual(const Math::Vec3& left, const Math::Vec3& right)
    {
        return NearlyEqual(left.x, right.x) && NearlyEqual(left.y, right.y) &&
            NearlyEqual(left.z, right.z);
    }

    MeshSpecification MakeMeshSpecification(
        const std::array<Vertex, 3>& vertices,
        const std::array<u32, 3>& indices,
        MeshAssetId assetId)
    {
        MeshSpecification specification;
        specification.vertexData = vertices.data();
        specification.vertexDataSize = sizeof(vertices);
        specification.vertexCount = static_cast<u32>(vertices.size());
        specification.layout = {
            {ShaderDataType::Float3, "Position"},
            {ShaderDataType::Float4, "Color"}};
        specification.indexData = indices.data();
        specification.indexCount = static_cast<u32>(indices.size());
        specification.assetId = assetId;
        return specification;
    }
}

int main()
{
    using namespace Pyramid;

    Tests::TestGraphicsDevice device;
    device.shaderFactory = []() { return std::make_shared<TestShader>(); };
    ResourceRegistry registry(device);

    const std::array<Vertex, 3> vertices = {
        Vertex{-2.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
        Vertex{3.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
        Vertex{0.0f, 4.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f}};
    const std::array<u32, 3> indices = {0, 1, 2};

    const MeshHandle meshHandle = registry.AcquireMesh(MakeMeshSpecification(
        vertices,
        indices,
        MeshAssetId::FromString("scene/player-mesh")));

    ShaderProgramSpecification shaderSpecification;
    shaderSpecification.vertexSource = "void main(){}";
    shaderSpecification.fragmentSource = "void main(){}";
    shaderSpecification.assetId = ShaderAssetId::FromString("scene/player-shader");
    const ShaderHandle shaderHandle = registry.AcquireShader(shaderSpecification);

    MaterialSpecification materialSpecification;
    materialSpecification.shader = registry.Resolve(shaderHandle);
    materialSpecification.assetId = MaterialAssetId::FromString("scene/player-material");
    const MaterialHandle materialHandle = registry.AcquireMaterial(materialSpecification);

    if (!meshHandle || !shaderHandle || !materialHandle)
    {
        return Fail("resource setup failed");
    }

    ResourceManifest manifest;
    if (!manifest.Add("z.mesh", meshHandle) || !manifest.Add("a.mesh", meshHandle) ||
        !manifest.Add("player.material", materialHandle))
    {
        return Fail("resource manifest setup failed");
    }

    Scene scene("Scene\tOne\nEncoded");
    auto first = std::make_shared<RenderObject>();
    first->name = "Player\nObject";
    first->position = Math::Vec3(10.0f, -2.5f, 7.0f);
    first->rotation = Math::Quat::FromAxisAngle(Math::Vec3::Up, Math::Radians(35.0f));
    first->scale = Math::Vec3(2.0f, 3.0f, 4.0f);
    first->visible = false;
    first->castShadows = false;
    first->receiveShadows = true;
    first->SetLocalBounds(Math::Vec3(-3.0f, -2.0f, -1.0f), Math::Vec3(4.0f, 5.0f, 6.0f));
    if (!first->SetMeshHandle(meshHandle, registry) ||
        !first->SetMaterialHandle(materialHandle, registry))
    {
        return Fail("handle-backed object setup failed");
    }
    scene.AddRenderObject(first);

    auto second = std::make_shared<RenderObject>();
    second->name = "Direct owner";
    second->mesh = registry.Resolve(meshHandle);
    second->material = registry.Resolve(materialHandle);
    second->position = Math::Vec3(-1.0f, 2.0f, 3.0f);
    scene.AddRenderObject(second);

    const SceneSerializationResult serialized =
        SceneSerializer::Serialize(scene, manifest, registry);
    const SceneSerializationResult serializedAgain =
        SceneSerializer::Serialize(scene, manifest, registry);
    if (!serialized.Succeeded() || serialized.serializedObjects != 2 ||
        serialized.text != serializedAgain.text ||
        serialized.text.find("PYRAMID_SCENE\t1\n") != 0 ||
        serialized.text.find("\ta.mesh\tplayer.material\t") == std::string::npos ||
        serialized.text.find("\tz.mesh\t") != std::string::npos)
    {
        return Fail("deterministic serialization or manifest-key selection failed");
    }

    const SceneDeserializationResult restored =
        SceneSerializer::Deserialize(serialized.text, manifest, registry);
    if (!restored.Succeeded() || restored.restoredObjects != 2 ||
        restored.scene->GetName() != scene.GetName() ||
        restored.scene->GetObjectCount() != 2)
    {
        return Fail("scene round trip failed");
    }

    const auto& restoredFirst = restored.scene->GetRenderObjects()[0];
    const auto& restoredSecond = restored.scene->GetRenderObjects()[1];
    if (!restoredFirst || restoredFirst->name != first->name ||
        !NearlyEqual(restoredFirst->position, first->position) ||
        !NearlyEqual(restoredFirst->scale, first->scale) ||
        restoredFirst->visible || restoredFirst->castShadows ||
        !restoredFirst->receiveShadows ||
        restoredFirst->GetBoundsMode() != RenderBoundsMode::Manual ||
        restoredFirst->meshHandle != meshHandle ||
        restoredFirst->materialHandle != materialHandle ||
        restoredFirst->mesh || restoredFirst->material)
    {
        return Fail("render-object state was not restored correctly");
    }

    Math::Vec3 restoredMin;
    Math::Vec3 restoredMax;
    restoredFirst->GetLocalBounds(restoredMin, restoredMax);
    if (!NearlyEqual(restoredMin, Math::Vec3(-3.0f, -2.0f, -1.0f)) ||
        !NearlyEqual(restoredMax, Math::Vec3(4.0f, 5.0f, 6.0f)) ||
        restoredSecond->GetBoundsMode() != RenderBoundsMode::Automatic ||
        restoredSecond->meshHandle != meshHandle ||
        restoredSecond->materialHandle != materialHandle)
    {
        return Fail("bounds mode or direct-resource conversion failed");
    }

    Scene invalidNameScene(std::string("bad\0scene", 9));
    const auto invalidName =
        SceneSerializer::Serialize(invalidNameScene, manifest, registry);
    if (invalidName.Succeeded() || invalidName.diagnostics.empty() ||
        invalidName.diagnostics.front().code !=
            SceneSerializationDiagnosticCode::InvalidNameEncoding)
    {
        return Fail("invalid scene names should be rejected before serialization");
    }

    ResourceManifest incompleteManifest;
    incompleteManifest.Add("player.material", materialHandle);
    const auto missingManifestSerialization =
        SceneSerializer::Serialize(scene, incompleteManifest, registry);
    if (missingManifestSerialization.Succeeded() ||
        missingManifestSerialization.diagnostics.empty() ||
        !missingManifestSerialization.text.empty())
    {
        return Fail("serialization should reject resources absent from the manifest");
    }

    ResourceManifest staleManifest;
    staleManifest.Add(
        "a.mesh",
        MeshHandle::FromParts(
            meshHandle.GetAssetId(),
            meshHandle.GetGeneration() + 1));
    staleManifest.Add("player.material", materialHandle);
    const auto stale = SceneSerializer::Deserialize(serialized.text, staleManifest, registry);
    if (stale.Succeeded() || stale.scene || stale.staleGenerations != 2 ||
        stale.diagnostics.empty())
    {
        return Fail("stale manifest generations were not rejected transactionally");
    }

    ResourceManifest missingManifest;
    missingManifest.Add(
        "a.mesh",
        MeshHandle::FromParts(MeshAssetId::FromString("scene/missing"), 1));
    missingManifest.Add("player.material", materialHandle);
    const auto missing = SceneSerializer::Deserialize(serialized.text, missingManifest, registry);
    if (missing.Succeeded() || missing.scene || missing.missingAssets != 2)
    {
        return Fail("missing registry assets were not diagnosed");
    }

    ResourceManifest wrongTypeManifest;
    wrongTypeManifest.Add("a.mesh", shaderHandle);
    wrongTypeManifest.Add("player.material", materialHandle);
    const auto wrongType =
        SceneSerializer::Deserialize(serialized.text, wrongTypeManifest, registry);
    if (wrongType.Succeeded() || wrongType.scene || wrongType.diagnostics.empty() ||
        wrongType.diagnostics.front().code !=
            SceneSerializationDiagnosticCode::ResourceTypeMismatch)
    {
        return Fail("resource type mismatch was not diagnosed");
    }

    const std::string unsupported =
        "PYRAMID_SCENE\t2\n"
        "scene\t-\n";
    const auto unsupportedResult =
        SceneSerializer::Deserialize(unsupported, manifest, registry);
    if (unsupportedResult.Succeeded() || unsupportedResult.scene ||
        unsupportedResult.diagnostics.empty() ||
        unsupportedResult.diagnostics.front().code !=
            SceneSerializationDiagnosticCode::UnsupportedVersion)
    {
        return Fail("unsupported version was not rejected");
    }

    const std::size_t objectLineStart = serialized.text.find("object\t");
    const std::size_t objectLineEnd = serialized.text.find('\n', objectLineStart);
    const std::string objectLine = serialized.text.substr(
        objectLineStart,
        objectLineEnd - objectLineStart + 1);
    const std::string duplicate = serialized.text + objectLine;
    const auto duplicateResult =
        SceneSerializer::Deserialize(duplicate, manifest, registry);
    if (duplicateResult.Succeeded() || duplicateResult.scene ||
        duplicateResult.diagnostics.empty())
    {
        return Fail("duplicate object keys were not rejected");
    }

    std::cout << "Scene serialization tests passed\n";
    return EXIT_SUCCESS;
}
