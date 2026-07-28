#include <Pyramid/Font/Font.hpp>

#include <cstdlib>
#include <iostream>

int main()
{
    Pyramid::Font::FontFace face;
    if (face.IsValid())
    {
        return EXIT_FAILURE;
    }

    Pyramid::Font::BakeOptions options;
    if (!options.IsValid())
    {
        return EXIT_FAILURE;
    }

    std::cout << "Pyramid::Font package is linked and operational\n";
    return EXIT_SUCCESS;
}
