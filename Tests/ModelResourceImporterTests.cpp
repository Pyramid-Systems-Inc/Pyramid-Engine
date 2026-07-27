#include <Pyramid/Graphics/Model/ModelResourceImporter.hpp>
#include <Pyramid/Graphics/Geometry/MeshCache.hpp>
#include <Pyramid/Model/ObjImporter.hpp>
#include <Pyramid/Util/Log.hpp>

#include "TestGraphicsDevice.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
    int Fail(const char* message)
    {
        std::cerr << "ModelResourceImporterTests failure: " << message << '\n';
        return EXIT_FAILURE;
    }

    Pyramid::Model::ImportedModel MakeTwoPrimitiveModel()
    {
        Pyramid::Model::ObjImportRequest request;
        request.sourcePath = "tests/two-primitives.obj";
        request.objText = R"OBJ(
o First
v 0 0 0
v 1 0 0
v 0 1 0
v 2 0 0
v 3 0 0
v 2 1 0
f 1 2 3
o Second
f 4 5 6
)OBJ";
        return Pyramid::Model::ObjImporter::Import(request);
    }

    Pyramid::Model::ImportedModel MakeSinglePrimitiveModel()
    {
        Pyramid::Model::ObjImportRequest request;
        request.sourcePath = "tests/one-primitive.obj";
        request.objText = R"OBJ(
o First
v 0 0 0
v 1 0 0
v 0 1 0
f 1 2 3
)OBJ";
        return Pyramid::Model::ObjImporter::Import(request);
    }
}

int main()
{
    Pyramid::Util::Logger::GetInstance().EnableConsole(false);
    Pyramid::Tests::TestGraphicsDevice device;
    Pyramid::MeshCache cache(device);

    const auto model = MakeTwoPrimitiveModel();
    if (!model.IsValid())
    {
        return Fail("test model did not parse");
    }

    Pyramid::ModelMeshImportOptions options;
    options.assetPrefix = "tests/imported";
    const auto imported = Pyramid::ModelResourceImporter::UploadMeshes(
        cache,
        model,
        options);
    if (!imported.IsSuccess() || imported.meshes.size() != 2 ||
        device.vertexBufferCreations != 2 || device.indexBufferCreations != 2)
    {
        return Fail("valid model primitives were not uploaded");
    }
    for (const auto& resource : imported.meshes)
    {
        const auto mesh = cache.Find(resource.mesh.GetAssetId());
        if (!mesh || mesh->GetVertexCount() != 3 || mesh->GetIndexCount() != 3)
        {
            return Fail("imported mesh handle did not resolve correctly");
        }
    }

    const auto importedAgain = Pyramid::ModelResourceImporter::UploadMeshes(
        cache,
        model,
        options);
    if (!importedAgain.IsSuccess() || device.vertexBufferCreations != 2)
    {
        return Fail("repeat import did not reuse resident meshes");
    }

    auto conflictingModel = model;
    conflictingModel.primitives[0].vertices[0].position.x = -2.0f;
    const auto conflict = Pyramid::ModelResourceImporter::UploadMeshes(
        cache,
        conflictingModel,
        options);
    if (conflict.IsSuccess() || conflict.GetErrorCount() == 0 ||
        cache.Find(imported.meshes[0].mesh.GetAssetId()) == nullptr)
    {
        return Fail("stable identifier conflict was not rejected transactionally");
    }

    cache.Clear();
    device.vertexBufferCreations = 0;
    device.indexBufferCreations = 0;

    const auto onePrimitive = MakeSinglePrimitiveModel();
    Pyramid::ModelMeshImportOptions preexistingOptions;
    preexistingOptions.assetPrefix = "tests/preexisting";
    const auto preexisting = Pyramid::ModelResourceImporter::UploadMeshes(
        cache,
        onePrimitive,
        preexistingOptions);
    if (!preexisting.IsSuccess() || cache.GetResidentCount() != 1)
    {
        return Fail("preexisting mesh fixture could not be published");
    }

    device.failVertexBufferCreationAt = device.vertexBufferCreations + 1;
    Pyramid::ModelMeshImportOptions transactionOptions;
    transactionOptions.assetPrefix = "tests/transaction";
    const auto failed = Pyramid::ModelResourceImporter::UploadMeshes(
        cache,
        model,
        transactionOptions);
    if (failed.IsSuccess() || !failed.meshes.empty() || failed.GetErrorCount() == 0)
    {
        return Fail("graphics allocation failure did not fail the complete import");
    }

    const auto transactionFirst = Pyramid::MeshAssetId::FromString(
        "tests/transaction/primitive/0/First");
    const auto transactionSecond = Pyramid::MeshAssetId::FromString(
        "tests/transaction/primitive/1/Second");
    if (cache.Contains(transactionFirst) ||
        cache.Contains(transactionSecond) ||
        cache.Find(preexisting.meshes[0].mesh.GetAssetId()) == nullptr ||
        cache.GetResidentCount() != 1)
    {
        return Fail("failed import did not roll back only its own aliases and geometry");
    }

    Pyramid::Model::ImportedModel invalid;
    invalid.sourcePath = "invalid.obj";
    const auto rejected = Pyramid::ModelResourceImporter::UploadMeshes(cache, invalid);
    if (rejected.IsSuccess() || rejected.GetErrorCount() == 0)
    {
        return Fail("invalid CPU model was accepted for upload");
    }

    return EXIT_SUCCESS;
}
