#ifndef COMMON_IMAGE_IO_H_
#define COMMON_IMAGE_IO_H_

typedef struct {
  int width;
  int height;
  int channels;
  unsigned char* data;
} ImageBuffer;

int image_load_png(const char* path, ImageBuffer* out_image);
int image_allocate_like(const ImageBuffer* source, ImageBuffer* out_image);
int image_save_png(const char* path, const ImageBuffer* image);
void image_free(ImageBuffer* image);

#endif
