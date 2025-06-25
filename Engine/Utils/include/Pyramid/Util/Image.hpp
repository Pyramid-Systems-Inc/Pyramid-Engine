#pragma once

#include <string>

namespace Pyramid
{
    namespace Util
    {

        // A struct to hold the loaded image data.
        // The user of this struct is responsible for freeing the data using Image::Free().
        struct ImageData
        {
            unsigned char *Data = nullptr;
            int Width = 0;
            int Height = 0;
            int Channels = 0;
        };

        // A static utility class for image operations.
        class Image
        {
        public:
            // Loads an image from the given file path.
            // Returns an ImageData struct. If loading fails, Data will be nullptr.
            static ImageData Load(const std::string &filepath);

            // Frees the memory allocated by the Load function.
            static void Free(unsigned char *data);
        };

    }
} // namespace Pyramid::Util