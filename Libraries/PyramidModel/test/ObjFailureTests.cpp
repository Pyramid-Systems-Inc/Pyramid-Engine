#include <Pyramid/Model/ObjImporter.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
    int Fail(const char* message)
    {
        std::cerr << "ObjFailureTests failure: " << message << '\n';
        return EXIT_FAILURE;
    }

    bool ContainsDiagnostic(
        const Pyramid::Model::ImportedModel& model,
        const std::string& text)
    {
        for (const auto& diagnostic : model.diagnostics)
        {
            if (diagnostic.message.find(text) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }
}

int main()
{
    using Pyramid::Model::ObjImportOptions;
    using Pyramid::Model::ObjImportRequest;
    using Pyramid::Model::ObjImporter;
    using Pyramid::Model::ObjMaterialLibrarySource;

    ObjImportRequest request;
    request.sourcePath = "missing.obj";
    request.objText = "mtllib missing.mtl\nv 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n";
    auto result = ObjImporter::Import(request);
    if (!result.HasErrors() || result.IsValid() ||
        !ContainsDiagnostic(result, "material library was not provided"))
    {
        return Fail("missing declared material library was not rejected");
    }

    ObjImportOptions permissive;
    permissive.requireDeclaredMaterialLibraries = false;
    result = ObjImporter::Import(request, permissive);
    if (!result.IsValid() || result.GetWarningCount() == 0)
    {
        return Fail("permissive missing-material mode did not preserve valid geometry");
    }

    request.objText = "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 0 2 3\n";
    result = ObjImporter::Import(request);
    if (!result.HasErrors() || !ContainsDiagnostic(result, "position index"))
    {
        return Fail("zero OBJ index was accepted");
    }

    request.objText = "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 9\n";
    result = ObjImporter::Import(request);
    if (!result.HasErrors() || !ContainsDiagnostic(result, "out of range"))
    {
        return Fail("out-of-range OBJ index was accepted");
    }

    request.objText = "v nan 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n";
    result = ObjImporter::Import(request);
    if (!result.HasErrors() || !ContainsDiagnostic(result, "finite position"))
    {
        return Fail("non-finite position was accepted");
    }

    request.objText = "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n";
    ObjImportOptions limited;
    limited.limits.maximumVertices = 2;
    result = ObjImporter::Import(request, limited);
    if (!result.HasErrors() || !ContainsDiagnostic(result, "vertex count exceeds"))
    {
        return Fail("vertex allocation limit was not enforced");
    }

    limited = {};
    limited.limits.maximumObjBytes = 8;
    result = ObjImporter::Import(request, limited);
    if (!result.HasErrors() || !ContainsDiagnostic(result, "size limit"))
    {
        return Fail("OBJ byte limit was not enforced");
    }

    request.objText = "mtllib duplicate.mtl\nv 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n";
    request.materialLibraries = {ObjMaterialLibrarySource{
        "duplicate.mtl",
        "newmtl Shared\nKd 1 0 0\nnewmtl Shared\nKd 0 1 0\n"}};
    result = ObjImporter::Import(request);
    if (!result.HasErrors() || !ContainsDiagnostic(result, "duplicate material"))
    {
        return Fail("duplicate material names were accepted");
    }

    request.materialLibraries.clear();
    request.objText = "v 0 0 0\nv 1 0 0\nv 0 1 0\n";
    result = ObjImporter::Import(request);
    if (!result.HasErrors() || !ContainsDiagnostic(result, "no renderable faces"))
    {
        return Fail("OBJ without faces was accepted");
    }

    request.objText = "v 0 0 0\nv 1 0 0\nv 0 1 0\nusemtl Unknown\nf 1 2 3\n";
    result = ObjImporter::Import(request);
    if (!result.IsValid() || result.GetWarningCount() == 0 ||
        result.primitives[0].materialIndex != -1)
    {
        return Fail("unknown material did not produce a fallback warning");
    }

    request.objText = "v invalid 0 0\nf 1 2 3\n";
    limited = {};
    limited.limits.maximumDiagnostics = 0;
    result = ObjImporter::Import(request, limited);
    if (!result.HasErrors() || result.diagnostics.size() != 1)
    {
        return Fail("zero diagnostic limit permitted a silent invalid import");
    }

    return EXIT_SUCCESS;
}
