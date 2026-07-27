#include <Pyramid/Model/ObjImporter.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace
{
    int Fail(const char* message)
    {
        std::cerr << "ObjFileTests failure: " << message << '\n';
        return EXIT_FAILURE;
    }

    bool WriteText(const std::filesystem::path& path, const std::string& text)
    {
        std::ofstream stream(path, std::ios::binary);
        stream << text;
        return static_cast<bool>(stream);
    }
}

int main()
{
    namespace fs = std::filesystem;
    using Pyramid::Model::ObjImporter;

    const fs::path root = fs::temp_directory_path() / "pyramid-model-file-test";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root / "materials", ignored);
    fs::create_directories(root / "textures", ignored);

    const fs::path objPath = root / "sample.obj";
    const fs::path materialPath = root / "materials" / "sample material.mtl";
    if (!WriteText(
            objPath,
            "mtllib \"materials/sample material.mtl\"\n"
            "v 0 0 0\nv 1 0 0\nv 0 1 0\n"
            "usemtl Surface\nf 1 2 3\n") ||
        !WriteText(
            materialPath,
            "newmtl Surface\nKd 0.2 0.4 0.6\n"
            "map_Kd \"../textures/diffuse map.png\"\n"))
    {
        fs::remove_all(root, ignored);
        return Fail("temporary fixture could not be written");
    }

    const auto model = ObjImporter::ImportFile(objPath.generic_string());
    const std::string expectedTexture =
        (root / "textures" / "diffuse map.png").lexically_normal().generic_string();
    if (!model.IsValid() || model.materials.size() != 1 ||
        model.primitives[0].materialIndex != 0 ||
        model.materials[0].diffuseTexture != expectedTexture)
    {
        fs::remove_all(root, ignored);
        return Fail("file import or quoted relative dependency resolution failed");
    }

    const auto missing = ObjImporter::ImportFile((root / "missing.obj").generic_string());
    if (!missing.HasErrors() || missing.diagnostics.empty())
    {
        fs::remove_all(root, ignored);
        return Fail("missing OBJ file did not return an explicit error");
    }

    fs::remove_all(root, ignored);
    return EXIT_SUCCESS;
}
