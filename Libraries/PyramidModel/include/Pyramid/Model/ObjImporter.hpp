#pragma once

#include <Pyramid/Model/Model.hpp>

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace Pyramid::Model
{
    struct ModelImportLimits
    {
        std::size_t maximumObjBytes = 64u * 1024u * 1024u;
        std::size_t maximumMaterialBytes = 16u * 1024u * 1024u;
        u32 maximumPositions = 1'000'000;
        u32 maximumTexCoords = 1'000'000;
        u32 maximumNormals = 1'000'000;
        u32 maximumVertices = 4'000'000;
        u32 maximumIndices = 12'000'000;
        u32 maximumPrimitives = 4096;
        u32 maximumMaterials = 4096;
        u32 maximumFaceVertices = 4096;
        u32 maximumDiagnostics = 4096;
    };

    struct ObjMaterialLibrarySource
    {
        std::string sourcePath;
        std::string text;
    };

    struct ObjImportRequest
    {
        std::string sourcePath;
        std::string objText;
        std::vector<ObjMaterialLibrarySource> materialLibraries;
    };

    struct ObjImportOptions
    {
        bool generateMissingNormals = true;
        bool flipTexCoordV = false;
        bool requireDeclaredMaterialLibraries = true;
        ModelImportLimits limits;
    };

    class ObjImporter final
    {
    public:
        static ImportedModel Import(
            const ObjImportRequest& request,
            const ObjImportOptions& options = {});

        static ImportedModel ImportFile(
            const std::string& filepath,
            const ObjImportOptions& options = {});
    };
}
