#include <Pyramid/Model/ObjImporter.hpp>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
    bool NearlyEqual(float left, float right, float epsilon = 0.0001f)
    {
        return std::fabs(left - right) <= epsilon;
    }

    int Fail(const char* message)
    {
        std::cerr << "ObjImportTests failure: " << message << '\n';
        return EXIT_FAILURE;
    }
}

int main()
{
    using Pyramid::Model::ObjImportOptions;
    using Pyramid::Model::ObjImportRequest;
    using Pyramid::Model::ObjImporter;
    using Pyramid::Model::ObjMaterialLibrarySource;

    ObjImportRequest request;
    request.sourcePath = "Assets/Models/colored.obj";
    request.objText = R"OBJ(
mtllib colored.mtl
o Board
g Front
v -1 -1 0 1 0 0
v  1 -1 0 0 1 0
v  1  1 0 0 0 1
v -1  1 0 1 1 1
vt 0 0
vt 1 0
vt 1 1
vt 0 1
vn 0 0 2
usemtl Red
f 1/1/1 2/2/1 3/3/1 4/4/1
g Back
usemtl Blue
f -4/1/1 -3/2/1 -2/3/1
)OBJ";
    request.materialLibraries.push_back(ObjMaterialLibrarySource{
        "Assets/Models/colored.mtl",
        R"MTL(
newmtl Red
Ka 0.1 0.2 0.3
Kd 0.8 0.1 0.2
Ks 0.3 0.4 0.5
Ns 32
d 0.75
illum 2
map_Kd -o 0.25 "../Textures/red diffuse.png"

newmtl Blue
Kd 0.1 0.2 0.9
Tr 0.25
)MTL"});

    const auto model = ObjImporter::Import(request);
    if (!model.IsValid() || model.GetErrorCount() != 0 || model.materials.size() != 2 ||
        model.primitives.size() != 2)
    {
        return Fail("valid OBJ/MTL input was not imported");
    }

    const auto& red = model.materials[0];
    if (red.name != "Red" || !NearlyEqual(red.diffuseColor.x, 0.8f) ||
        !NearlyEqual(red.specularExponent, 32.0f) || !NearlyEqual(red.opacity, 0.75f) ||
        red.illuminationModel != 2 ||
        red.diffuseTexture != "Assets/Textures/red diffuse.png")
    {
        return Fail("MTL properties or relative texture path are incorrect");
    }
    if (!NearlyEqual(model.materials[1].opacity, 0.75f))
    {
        return Fail("Tr transparency was not converted to opacity");
    }

    const auto& front = model.primitives[0];
    if (front.name != "Board/Front:Red" || front.materialIndex != 0 ||
        front.vertices.size() != 4 || front.indices.size() != 6 ||
        !front.hasNormals || !front.hasTexCoords)
    {
        return Fail("quad triangulation, deduplication, or primitive metadata is incorrect");
    }
    if (!NearlyEqual(front.boundsMin.x, -1.0f) || !NearlyEqual(front.boundsMin.y, -1.0f) ||
        !NearlyEqual(front.boundsMax.x, 1.0f) || !NearlyEqual(front.boundsMax.y, 1.0f) ||
        !NearlyEqual(front.vertices[0].normal.z, 1.0f) ||
        !NearlyEqual(front.vertices[0].color.x, 1.0f) ||
        !NearlyEqual(front.vertices[1].color.y, 1.0f))
    {
        return Fail("bounds, normalized source normals, or vertex colors are incorrect");
    }

    ObjImportRequest generatedRequest;
    generatedRequest.sourcePath = "generated.obj";
    generatedRequest.objText = R"OBJ(
v 0 0 0
v 1 0 0
v 1 1 0
v 0 1 0
vt 0 0
vt 1 0
vt 1 1
vt 0 1
s smooth
f -4/-4 -3/-3 -2/-2
f -4/-4 -2/-2 -1/-1
)OBJ";

    ObjImportOptions generatedOptions;
    generatedOptions.flipTexCoordV = true;
    const auto generated = ObjImporter::Import(generatedRequest, generatedOptions);
    if (!generated.IsValid() || generated.primitives.size() != 1 ||
        generated.primitives[0].vertices.size() != 4 ||
        !generated.primitives[0].hasNormals || !generated.primitives[0].hasTexCoords)
    {
        return Fail("negative indices or smooth generated normals are incorrect");
    }
    for (const auto& vertex : generated.primitives[0].vertices)
    {
        if (!NearlyEqual(vertex.normal.x, 0.0f) || !NearlyEqual(vertex.normal.y, 0.0f) ||
            !NearlyEqual(vertex.normal.z, 1.0f))
        {
            return Fail("generated smooth normal is incorrect");
        }
    }
    if (!NearlyEqual(generated.primitives[0].vertices[0].texCoord.y, 1.0f))
    {
        return Fail("texture-coordinate V flipping was not applied");
    }

    generatedRequest.objText = R"OBJ(
v 0 0 0
v 1 0 0
v 1 1 0
v 0 1 0
s off
f 1 2 3
f 1 3 4
)OBJ";
    const auto hardEdges = ObjImporter::Import(generatedRequest);
    if (!hardEdges.IsValid() || hardEdges.primitives[0].vertices.size() != 6 ||
        hardEdges.primitives[0].hasTexCoords)
    {
        return Fail("smoothing-off faces were not split into hard-edge vertices");
    }

    return EXIT_SUCCESS;
}
