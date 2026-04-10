#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <stddef.h>
#include <string.h>

int main(int argc, char** argv) {
  int lhs_width = 0;
  int lhs_height = 0;
  int lhs_channels = 0;
  int rhs_width = 0;
  int rhs_height = 0;
  int rhs_channels = 0;
  unsigned char* lhs = NULL;
  unsigned char* rhs = NULL;
  size_t byte_count;

  if (argc != 3) {
    return 2;
  }

  lhs = stbi_load(argv[1], &lhs_width, &lhs_height, &lhs_channels, 0);
  rhs = stbi_load(argv[2], &rhs_width, &rhs_height, &rhs_channels, 0);
  if (lhs == NULL || rhs == NULL) {
    stbi_image_free(lhs);
    stbi_image_free(rhs);
    return 1;
  }

  if (lhs_width != rhs_width || lhs_height != rhs_height || lhs_channels != rhs_channels) {
    stbi_image_free(lhs);
    stbi_image_free(rhs);
    return 1;
  }

  byte_count = (size_t)lhs_width * (size_t)lhs_height * (size_t)lhs_channels;
  if (memcmp(lhs, rhs, byte_count) != 0) {
    stbi_image_free(lhs);
    stbi_image_free(rhs);
    return 1;
  }

  stbi_image_free(lhs);
  stbi_image_free(rhs);
  return 0;
}
