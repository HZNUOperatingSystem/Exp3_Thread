#include <thread>
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
  const int num_threads = 4;

  std::vector<std::thread> threads;
  threads.reserve(num_threads);

  for (int i = 0; i < num_threads; ++i) {
    const int y_begin = height * i / num_threads;
    const int y_end = height * (i + 1) / num_threads;
    threads.emplace_back([=, &dst]() {
      bilateral_filter_rows(src, dst.data(), width, height, channels, y_begin, y_end, params);
    });
  }

  for (std::thread& thread : threads) {
    thread.join();
  }

  const int ok =
      stbi_write_png("output.png", width, height, channels, dst.data(), width * channels);
  stbi_image_free(src);
  return ok ? 0 : 1;
}
