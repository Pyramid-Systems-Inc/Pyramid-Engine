#include <Pyramid/Model/ObjImporter.hpp>

#include <iostream>

int main()
{
    Pyramid::Model::ObjImportRequest request;
    request.sourcePath = "consumer.obj";
    request.objText =
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 0 1 0\n"
        "f 1 2 3\n";

    const auto model = Pyramid::Model::ObjImporter::Import(request);
    std::cout << "Pyramid::Model package is linked and operational\n";
    return model.IsValid() && model.primitives.size() == 1 ? 0 : 1;
}
