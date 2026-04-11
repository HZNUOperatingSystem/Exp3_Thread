#include "common/image_io.h"

#include <stddef.h>
#include <stdlib.h>

#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

int image_load_png(const char* path, ImageBuffer* out_image) {
  if (path == NULL || out_image == NULL) {
    return -1;
  }

  out_image->data = stbi_load(path, &out_image->width, &out_image->height, &out_image->channels, 0);
  if (out_image->data == NULL) {
    out_image->width = 0;
    out_image->height = 0;
    out_image->channels = 0;
    return -1;
  }

  return 0;
}

int image_allocate_like(const ImageBuffer* source, ImageBuffer* out_image) {
  size_t byte_count;

  if (source == NULL || out_image == NULL || source->width <= 0 || source->height <= 0 ||
      source->channels <= 0) {
    return -1;
  }

  byte_count = (size_t)source->width * (size_t)source->height * (size_t)source->channels;
  out_image->width = source->width;
  out_image->height = source->height;
  out_image->channels = source->channels;
  out_image->data = (unsigned char*)malloc(byte_count);
  if (out_image->data == NULL) {
    out_image->width = 0;
    out_image->height = 0;
    out_image->channels = 0;
    return -1;
  }

  return 0;
}

int image_save_png(const char* path, const ImageBuffer* image) {
  if (path == NULL || image == NULL || image->data == NULL) {
    return -1;
  }

  if (!stbi_write_png(path, image->width, image->height, image->channels, image->data,
                      image->width * image->channels)) {
    return -1;
  }

  return 0;
}

void image_free(ImageBuffer* image) {
  if (image == NULL) {
    return;
  }

  if (image->data != NULL) {
    stbi_image_free(image->data);
  }

  image->width = 0;
  image->height = 0;
  image->channels = 0;
  image->data = NULL;
}
