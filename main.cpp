#include <cstddef>
#include <vector>

#include "bilateral_filter.h"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

int main() {
  int width = 0;
  int height = 0;
  int channels = 0;

  unsigned char* src = stbi_load("input.png", &width, &height, &channels, 0);
  if (src == nullptr) {
    return 1;
  }

  int image_size = width * height * channels;
  std::vector<unsigned char> dst(image_size);
  BilateralFilterParams params;

  // TODO:
  // 1. Decide how many threads to create.
  // 2. Split the image by rows.
  // 3. Let each thread call bilateral_filter_rows(...) on its own row range.
  // 4. Join all threads before writing output.png.
  bilateral_filter_image(src, dst.data(), width, height, channels, params);

  const int ok =
      stbi_write_png("output.png", width, height, channels, dst.data(), width * channels);
  stbi_image_free(src);
  return ok ? 0 : 1;
}
