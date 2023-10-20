#include "common.h" 

#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace utils {
  std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
      return {};
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
  }

  void Image::read(const std::string& filename) {
    pixels = stbi_load(filename.data(), &width, &height, &texChannels, STBI_rgb_alpha);
    size = width * height * texChannels;
  }
}