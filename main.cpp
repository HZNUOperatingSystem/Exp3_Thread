#include <thread>
#include <vector>

#include "bilateral_filter.h"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

void parallel_filter(const unsigned char* src, unsigned char* dst, int width, int height,
                     int channels, int num_threads, const BilateralFilterParams& params) {
  if (num_threads <= 1 || height <= 1) {
    bilateral_filter_image(src, dst, width, height, channels, params);
    return;
  }

  int worker_count = num_threads;
  if (worker_count > height) {
    worker_count = height;
  }

  std::vector<std::thread> threads;
  threads.reserve(worker_count);

  for (int i = 0; i < worker_count; ++i) {
    const int y_begin = height * i / worker_count;
    const int y_end = height * (i + 1) / worker_count;
    threads.emplace_back([=, &dst]() {
      bilateral_filter_rows(src, dst, width, height, channels, y_begin, y_end, params);
    });
  }

  for (std::thread& thread : threads) {
    thread.join();
  }
}

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

  parallel_filter(src, dst.data(), width, height, channels, num_threads, params);

  const int ok =
      stbi_write_png("output.png", width, height, channels, dst.data(), width * channels);
  stbi_image_free(src);
  return ok ? 0 : 1;
}
