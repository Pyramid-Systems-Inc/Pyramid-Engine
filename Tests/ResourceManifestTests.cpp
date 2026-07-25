#include <Pyramid/Graphics/Geometry/Vertex.hpp>
#include <Pyramid/Graphics/Resources/ResourceManifest.hpp>
#include <Pyramid/Graphics/Resources/ResourceRegistry.hpp>

#include "TestGraphicsDevice.hpp"

#include <array>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace
{
    using namespace Pyramid;

    int Fail(const char* message)
    {
        std::cerr << "ResourceManifestTests failure: " << message << '\n';
        return EXIT_FAILURE;
    }

    MeshSpecification MakeMeshSpecification(
        const std::array<Vertex, 3>& vertices,
        const std::array<u32, 3>& indices,
        MeshAssetId assetId)
    {
        MeshSpecification specification;
        specification.vertexData = vertices.data();
        specification.vertexDataSize = static_cast<u32>(vertices.size() * sizeof(Vertex));
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

    ResourceManifest manifest;
    const auto meshId = MeshAssetId::FromString("manifest/player-mesh");
    const auto shaderId = ShaderAssetId::FromString("manifest/player-shader");
    const auto textureId = TextureAssetId::FromString("manifest/player-texture");
    const auto materialId = MaterialAssetId::FromString("manifest/player-material");

    const MeshHandle mesh = MeshHandle::FromParts(meshId, 7);
    const ShaderHandle shader = ShaderHandle::FromParts(shaderId, 3);
    const TextureHandle texture = TextureHandle::FromParts(textureId, 11);
    const MaterialHandle material = MaterialHandle::FromParts(materialId, 5);

    if (!manifest.Add("player.mesh", mesh) || !manifest.Add("player.shader", shader) ||
        !manifest.Add("player.texture", texture) || !manifest.Add("player.material", material) ||
        manifest.Add("player.mesh", mesh) || manifest.Add("invalid key", mesh) ||
        manifest.GetEntryCount() != 4)
    {
        return Fail("typed manifest insertion or validation failed");
    }

    if (manifest.GetMeshHandle("player.mesh") != mesh ||
        manifest.GetShaderHandle("player.shader") != shader ||
        manifest.GetTextureHandle("player.texture") != texture ||
        manifest.GetMaterialHandle("player.material") != material ||
        manifest.GetMeshHandle("player.shader"))
    {
        return Fail("typed manifest handle restoration failed");
    }

    const std::string serialized = manifest.Serialize();
    if (serialized.find("PYRAMID_RESOURCE_MANIFEST\t1\n") != 0 ||
        serialized.find("mesh\tplayer.mesh\t" + meshId.ToString() + "\t7\n") == std::string::npos)
    {
        return Fail("manifest serialization is malformed");
    }

    ResourceManifest parsed;
    std::vector<ResourceManifestDiagnostic> diagnostics;
    if (!ResourceManifest::Deserialize(serialized, parsed, diagnostics) ||
        !diagnostics.empty() || parsed.Serialize() != serialized ||
        parsed.GetMeshHandle("player.mesh") != mesh)
    {
        return Fail("manifest round trip failed");
    }

    ResourceManifest unchanged;
    unchanged.Add("keep.mesh", mesh);
    const std::string beforeFailure = unchanged.Serialize();
    const std::string invalid =
        "PYRAMID_RESOURCE_MANIFEST\t2\n"
        "mesh\tbad key\t00000000000000000000000000000000\t0\n";
    if (ResourceManifest::Deserialize(invalid, unchanged, diagnostics) ||
        diagnostics.empty() || unchanged.Serialize() != beforeFailure ||
        diagnostics.front().code != ResourceManifestDiagnosticCode::UnsupportedVersion)
    {
        return Fail("transactional parse or version diagnostics failed");
    }

    const std::string malformed =
        "PYRAMID_RESOURCE_MANIFEST\t1\n"
        "mesh\tduplicate\t" + meshId.ToString() + "\t1\n"
        "shader\tduplicate\t" + shaderId.ToString() + "\t1\n"
        "unknown\tbad\t" + shaderId.ToString() + "\t1\n";
    ResourceManifest rejected;
    if (ResourceManifest::Deserialize(malformed, rejected, diagnostics) ||
        diagnostics.size() != 2 || !rejected.Empty())
    {
        return Fail("entry diagnostics or transactional rejection failed");
    }

    Tests::TestGraphicsDevice device;
    ResourceRegistry registry(device);
    const std::array<Vertex, 3> vertices = {
        Vertex{-1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
        Vertex{1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
        Vertex{0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f}};
    const std::array<u32, 3> indices = {0, 1, 2};
    const auto liveId = MeshAssetId::FromString("manifest/live-mesh");
    const auto liveHandle = registry.AcquireMesh(MakeMeshSpecification(vertices, indices, liveId));
    if (!liveHandle)
    {
        return Fail("test mesh acquisition failed");
    }

    ResourceManifest restoreManifest;
    restoreManifest.Add("live.mesh", liveHandle);
    restoreManifest.Add("missing.shader", ShaderHandle::FromParts(shaderId, 1));
    restoreManifest.Add("stale.mesh", MeshHandle::FromParts(liveId, liveHandle.GetGeneration() + 1));

    const auto report = restoreManifest.Restore(registry);
    if (report.resolved != 1 || report.missingAssets != 1 ||
        report.staleGenerations != 1 || report.diagnostics.size() != 2 ||
        report.IsComplete())
    {
        return Fail("registry restoration diagnostics were incorrect");
    }

    if (!manifest.Remove("player.material") || manifest.Contains("player.material") ||
        manifest.Remove("player.material"))
    {
        return Fail("manifest removal failed");
    }

    std::cout << "Resource manifest tests passed\n";
    return EXIT_SUCCESS;
}
