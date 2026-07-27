#pragma once

#include <Pyramid/Core/Prerequisites.hpp>
#include <Pyramid/Math/Math.hpp>

#include <string>
#include <vector>

namespace Pyramid::Model
{
    enum class ImportDiagnosticSeverity : u8
    {
        Warning = 0,
        Error
    };

    struct ImportDiagnostic
    {
        ImportDiagnosticSeverity severity = ImportDiagnosticSeverity::Error;
        u32 line = 0;
        std::string source;
        std::string message;

        bool IsError() const { return severity == ImportDiagnosticSeverity::Error; }
    };

    struct ModelVertex
    {
        Math::Vec3 position;
        Math::Vec3 normal;
        Math::Vec2 texCoord;
        Math::Vec4 color = Math::Vec4(1.0f);
    };

    struct ImportedMaterial
    {
        std::string name;
        Math::Vec3 ambientColor = Math::Vec3(0.0f);
        Math::Vec3 diffuseColor = Math::Vec3(1.0f);
        Math::Vec3 specularColor = Math::Vec3(0.0f);
        f32 specularExponent = 0.0f;
        f32 opacity = 1.0f;
        i32 illuminationModel = 0;
        std::string diffuseTexture;
    };

    struct ImportedPrimitive
    {
        std::string name;
        i32 materialIndex = -1;
        std::vector<ModelVertex> vertices;
        std::vector<u32> indices;
        Math::Vec3 boundsMin;
        Math::Vec3 boundsMax;
        bool hasNormals = false;
        bool hasTexCoords = false;
    };

    struct ImportedModel
    {
        std::string sourcePath;
        std::vector<ImportedMaterial> materials;
        std::vector<ImportedPrimitive> primitives;
        std::vector<ImportDiagnostic> diagnostics;

        u32 GetErrorCount() const;
        u32 GetWarningCount() const;
        bool HasErrors() const { return GetErrorCount() != 0; }
        bool IsValid() const;
    };
}
