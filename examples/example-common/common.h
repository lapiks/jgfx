#pragma once

#include <vector>
#include <string>

namespace utils {
  std::vector<char> readFile(const std::string& filename);

  struct Image {
    void read(const std::string& filename);
    unsigned char* pixels = nullptr;
    uint64_t size = 0; // size in bytes
    int width = 0;
    int height = 0;
    int texChannels = 0;
  };
}